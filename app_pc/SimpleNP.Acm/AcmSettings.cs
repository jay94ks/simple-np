using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SimpleNP.Acm
{
    /// <summary>
    /// ACM device settings.
    /// </summary>
    public class AcmSettings
    {
        /// <summary>
        /// Port Number.
        /// </summary>
        [JsonProperty("port")]
        public string Port { get; set; } = "COM1";

        /// <summary>
        /// Baud Rate.
        /// </summary>
        [JsonProperty("baud_rate")]
        public int BaudRate { get; set; } = 115200;

        /// <summary>
        /// Parity bits.
        /// </summary>
        [JsonProperty("parity")]
        public Parity Parity { get; set; } = Parity.None;

        /// <summary>
        /// Data bits.
        /// </summary>
        [JsonProperty("data_bits")]
        public int DataBits { get; set; } = 8;

        /// <summary>
        /// Stop bits.
        /// </summary>
        [JsonProperty("stop_bits")]
        public StopBits StopBits { get; set; } = StopBits.One;

        /// <summary>
        /// Recognition information.
        /// </summary>
        [JsonProperty("recognition")]
        public AcmRecognitionInfo RecognitionInfo { get; set; } = null;

        /// <summary>
        /// Scan all ACM devices.
        /// </summary>
        /// <returns></returns>
        public static IEnumerable<AcmSettings> ScanAll()
        {
            var Devices = AcmDeviceScanner.Scan();
            foreach(var EachRI in AcmRecognitionInfo.WHITELIST)
            {
                var Matches = Devices.Where(X => X.Is(EachRI));
                foreach(var Each in Matches)
                {
                    yield return new AcmSettings
                    {
                        Port = Each.PortName,
                        RecognitionInfo = EachRI,
                    };
                }
            }
        }

        /// <summary>
        /// Test whether the settings are valid or not.
        /// </summary>
        /// <returns></returns>
        public bool IsValid()
        {
            if (string.IsNullOrWhiteSpace(Port))
                return false;

            if (StopBits == StopBits.None)
                return false;

            if (BaudRate <= 0 || DataBits <= 0)
                return false;

            return true;
        }

        /// <summary>
        /// Test whether two settings are same or not.
        /// </summary>
        /// <param name="Settings"></param>
        /// <returns></returns>
        public bool IsSame(AcmSettings Settings)
        {
            if (Settings is null)
                return false;

            if (Settings.Port != Port)
                return false;

            if (Settings.BaudRate != BaudRate)
                return false;

            if (Settings.Parity != Parity)
                return false;

            if (Settings.DataBits != DataBits)
                return false;

            if (Settings.StopBits != StopBits)
                return false;

            return true;
        }

        /// <summary>
        /// Clone the setting values.
        /// </summary>
        /// <returns></returns>
        public AcmSettings Clone() => new AcmSettings
        {
            Port = Port,
            BaudRate = BaudRate,
            Parity = Parity,
            DataBits = DataBits,
            StopBits = StopBits
        };
    }
}
