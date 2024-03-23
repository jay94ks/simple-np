using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SimpleNP.Acm.Drivers
{
    internal class AcmConstants
    {
        /// <summary>
        /// STX.
        /// </summary>
        public const byte STX = 0x02;

        /// <summary>
        /// ETX.
        /// </summary>
        public const byte ETX = 0x03;

        /// <summary>
        /// Compute checksum.
        /// Buffer region must include STX to ETX.
        /// </summary>
        /// <param name="Buffer"></param>
        /// <param name="Offset"></param>
        /// <param name="Length"></param>
        /// <returns></returns>
        public static byte Checksum(byte[] Buffer, int Offset, int Length)
        {
            ushort Sum = 0;
            for(int i = 0; i < Length; i++)
            {
                int Index = Offset + i;
                Sum += Buffer[Index];
            }

            return (byte)(Sum & 0xff);
        }
    }
}
