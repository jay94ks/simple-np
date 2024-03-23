using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Xml.Linq;
using static SimpleNP.Acm.Drivers.AcmConstants;

namespace SimpleNP.Acm.Drivers
{
    /// <summary>
    /// Keyboard Device instance.
    /// </summary>
    public class AcmDriver : IDisposable
    {
        private readonly AcmSettings m_Settings;
        private readonly CancellationTokenSource m_Disposing;
        private readonly Dictionary<AcmKeys, AcmKeyState> m_Keys;
        private readonly SemaphoreSlim m_Semaphore;
        private readonly IDisposable m_Monitor;
        private SerialPort m_COM;
        private bool m_Disposed = false;

        private byte[] m_Buffer;

        private int m_Stage = 0;
        private int m_Offset = 0;
        private int m_Remark = 0;
        private byte m_Cmd = 0;
        private byte[] m_Data = null;
        private int m_DataLen = 0;


        /// <summary>
        /// Initialize a new <see cref="AcmDriver"/> instance.
        /// </summary>
        /// <param name="Settings"></param>
        public AcmDriver(AcmSettings Settings)
        {
            m_Settings = Settings;
            m_Semaphore = new SemaphoreSlim(1);
            m_Keys = new Dictionary<AcmKeys, AcmKeyState>();
            m_Buffer = Array.Empty<byte>();

            if (Settings is null)
                throw new ArgumentNullException(nameof(Settings));

            if (Settings.IsValid() == false)
                throw new InvalidOperationException("the specified settings object is not valid.");

            Disposing = (m_Disposing = new()).Token;
            OnBytesReceived += OnKeyLevelEvent;
            try
            {
                m_COM = new SerialPort(
                    Settings.Port, Settings.BaudRate, Settings.Parity,
                    Settings.DataBits, Settings.StopBits);

                m_COM.DataReceived += OnDataReceived;
                m_COM.ErrorReceived += (s, e) =>
                {
                    Dispose();
                };
                m_Monitor = AcmDeviceInfo.Monitor(Settings.Port, OnMonitorCallback);
                m_COM.Open();
            }

            catch { Dispose(); }
        }

        /// <summary>
        /// Triggered when the <see cref="AcmDriver"/> is disposing.
        /// </summary>
        public CancellationToken Disposing { get; }

        /// <summary>
        /// Indicates whether the instance is alive or not.
        /// </summary>
        public bool IsAlive => Disposing.IsCancellationRequested == false;

        /// <inheritdoc/>
        public void Dispose()
        {
            SerialPort COM;
            lock(this)
            {
                if (m_Disposed)
                    return;

                COM = m_COM;

                m_Disposed = true;
                m_COM = null;
            }

            try { m_Disposing.Cancel(); } catch { }
            try { m_Monitor?.Dispose(); } catch { }
            try { m_Semaphore.Dispose(); } catch { }
            try { COM?.Dispose(); } catch { }
            try { m_Disposing.Dispose(); } catch { }
        }

        /// <summary>
        /// Called when the port state changed.
        /// </summary>
        /// <param name="Port"></param>
        /// <param name="Online"></param>
        private void OnMonitorCallback(string Port, bool Online)
        {
            if (Online)
                return;

            lock (this)
            {
                if (m_Disposed)
                    return;
            }

            Dispose();
        }

        /// <summary>
        /// Called when data received from ACM device.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnDataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            var Com = m_COM;
            if (Com is null)
                return;

            try
            {
                if (Com.BytesToRead <= 0)
                    return;

                var Data = new byte[Com.BytesToRead];
                var Length = Com.Read(Data, 0, Data.Length);
                if (Length <= 0)
                    return;

                // --> concatenate buffer.
                if (m_Buffer is null || m_Buffer.Length <= 0)
                {
                    if (Length != Data.Length)
                        Array.Resize(ref Data, Length);

                    m_Buffer = Data;
                }

                else
                {
                    Array.Resize(ref m_Buffer, m_Buffer.Length + Length);
                    Array.Copy(Data, 0, m_Buffer, m_Buffer.Length - Length, Length);
                }
            }

            catch
            {
                Dispose();
                return;
            }

            HandleBufferedBytes();
        }

        /// <summary>
        /// Handle buffered bytes.
        /// </summary>
        private void HandleBufferedBytes()
        {
            while(true)
            {
                int Stage = m_Stage;
                switch(Stage)
                {
                    case 0: // --> Wait STX.
                        while (DequeueFromBuffer(out var MaySTX))
                        {
                            if (MaySTX != STX)
                                continue;

                            // --> cut useless bytes from buffer.
                            //CutUselessBuffer();
                            m_Remark = m_Offset - 1;
                            m_Stage++;
                            break;
                        }
                        break;

                    case 1: // --> Wait CMD.
                        if (DequeueFromBuffer(out m_Cmd))
                        {
                            m_Stage++;
                        }

                        break;

                    case 2: // --> Wait LEN.
                        if (DequeueFromBuffer(out var Len))
                        {
                            if (Len > 16)
                                Len = 16;

                            m_Data = new byte[Len];
                            m_Stage++;
                            m_DataLen = 0;
                        }
                        break;

                    case 3: // --> Wait DATA.
                        while(m_DataLen < m_Data.Length)
                        {
                            if (DequeueFromBuffer(out var Temp) == false)
                                break;

                            m_Data[m_DataLen++] = Temp;
                            if (m_DataLen == m_Data.Length)
                            {
                                m_Stage++;
                                break;
                            }
                        }

                        break;

                    case 4: // --> Wait ETX.
                        if (DequeueFromBuffer(out var MayETX) == false)
                            break;

                        if (MayETX != ETX)
                        {
                            m_Stage = 0;

                            // --> reset if mismatch.
                            CutUselessBuffer();
                            break;
                        }

                        m_Stage++;
                        break;

                    case 5: // --> Wait CHK.
                        if (DequeueFromBuffer(out var MayCHK) == false)
                            break;
                        {
                            byte Chk = Checksum(m_Buffer, m_Remark, m_Offset - (m_Remark + 1));
                            if (Chk == MayCHK)
                            {
                                OnBytesReceived?.Invoke(this, m_Cmd, m_Data);
                            }

                            // --> reset state.
                            m_Offset--;
                            m_Stage = 0;

                        }
                        CutUselessBuffer();
                        break;

                    default:
                        m_Stage = 0;
                        CutUselessBuffer();
                        break;
                }

                if (Stage != m_Stage)
                    continue;

                break;
            }
        }

        /// <summary>
        /// Dequeue a byte from buffer.
        /// </summary>
        /// <param name="Byte"></param>
        /// <returns></returns>
        private bool DequeueFromBuffer(out byte Byte)
        {
            if (m_Offset < m_Buffer.Length)
            {
                Byte = m_Buffer[m_Offset++];
                return true;
            }

            Byte = 0;
            return false;
        }

        /// <summary>
        /// Cut useless byte from buffer.
        /// </summary>
        private void CutUselessBuffer()
        {
            if (m_Offset > 0)
            {
                m_Buffer = m_Buffer.Skip(m_Offset).ToArray();
                m_Offset = 0;
            }
        }

        /// <summary>
        /// Triggered when the data bytes received.
        /// </summary>
        internal event Action<AcmDriver, byte, byte[]> OnBytesReceived;

        /// <summary>
        /// Triggered when the key state changed.
        /// </summary>
        public event Action<AcmDriver, AcmKeys, AcmKeyState> OnKeyStateChanged;

        /// <summary>
        /// Emit `NOP` message and wait its result.
        /// </summary>
        /// <param name="Token"></param>
        /// <returns></returns>
        public Task<bool> PingAsync(CancellationToken Token =default) => EmitAsync(0, Array.Empty<byte>(), Token);  

        /// <summary>
        /// Emit the command and its data bytes asynchronously.
        /// </summary>
        /// <param name="Cmd"></param>
        /// <param name="Data"></param>
        /// <param name="Token"></param>
        /// <returns></returns>
        internal async Task<bool> EmitAsync(byte Cmd, ArraySegment<byte> Data, CancellationToken Token = default)
        {
            try { await m_Semaphore.WaitAsync(Token); }
            catch
            {
                return false;
            }

            try
            {
                var Com = m_COM;
                if (Com is null)
                    return false;

                var Bytes = Data.Take(16).ToArray();

                // STX CMD LEN DATA ETX CHK
                var Message = new byte[3 + 2 + Bytes.Length];

                // --> fill data field.
                Bytes.CopyTo(Message, 3);

                Message[0] = STX;
                Message[1] = Cmd;
                Message[2] = (byte) Bytes.Length;
                Message[Message.Length - 2] = ETX;
                Message[Message.Length - 1] = Checksum(Message, 0, Message.Length - 1);

                try { Com.Write(Bytes, 0, Bytes.Length); }
                catch
                {
                    Dispose();
                    return false;
                }

                return true;
            }

            finally
            {
                try { m_Semaphore.Release(); }
                catch
                {
                }
            }
        }

        /// <summary>
        /// Handle Key level event only.
        /// </summary>
        /// <param name="Acm"></param>
        /// <param name="Cmd"></param>
        /// <param name="Args"></param>
        private void OnKeyLevelEvent(AcmDriver Acm, byte Cmd, byte[] Args)
        {
            if (Cmd != 0xfe || Args.Length < 2)
                return;

            var Key = (AcmKeys) Args[0];
            var State = (AcmKeyState)Args[1];

            lock(m_Keys)
            {
                m_Keys[Key] = State;
            }

            OnKeyStateChanged?.Invoke(this, Key, State);
        }

    }

}
