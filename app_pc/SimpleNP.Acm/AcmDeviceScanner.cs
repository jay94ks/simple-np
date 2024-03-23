using System.Management;
using System.Text.RegularExpressions;

namespace SimpleNP.Acm
{
    /// <summary>
    /// Scans ACM devices and refine their informations into <see cref="AcmDeviceInfo"/> object.
    /// </summary>
    internal static class AcmDeviceScanner
    {
        private const string PATTERN_VID = @"VID_([0-9a-fA-F]{4})";
        private const string PATTERN_PID = @"PID_([0-9a-fA-F]{4})";
        private const string PATTERN_COM = @"[^(]*\(([^)]*)\)";
        private const string PNP_QUERY = "SELECT * FROM Win32_PnPEntity";

        /// <summary>
        /// Get the property value of management object.
        /// </summary>
        /// <param name="Object"></param>
        /// <param name="Name"></param>
        /// <returns></returns>
        private static string GetPropertyString(this ManagementBaseObject Object, string Name)
        {
            if (OperatingSystem.IsWindows())
            {
                var Obj = Object.GetPropertyValue(Name);
                if (Obj is null)
                    return string.Empty;

                return Obj.ToString();
            }

            return null;
        }

        /// <summary>
        /// Get the capture group's value.
        /// </summary>
        /// <param name="Match"></param>
        /// <param name="Number"></param>
        /// <returns></returns>
        private static string GetGroup(this Match Match, int Number)
        {
            if (Match.Groups.Count <= Number)
                return string.Empty;

            return Match.Groups[Number].Value ?? string.Empty;
        }

        /// <summary>
        /// Scan all usb serial ports.
        /// </summary>
        /// <returns></returns>
        public static IEnumerable<AcmDeviceInfo> Scan()
        {
            if (OperatingSystem.IsWindows())
            {
                using var Scanner = new ManagementObjectSearcher(PNP_QUERY);
                foreach (var Each in Scanner.Get().Cast<ManagementBaseObject>().ToArray())
                {
                    var Pdi = Each.GetPropertyString("PNPDeviceID");
                    var Pnp = Each.GetPropertyString("PNPClass");
                    var Cap = Each.GetPropertyString("Caption");

                    if (Pnp.Contains("Ports") == false)
                        continue;

                    var MatVID = Regex.Match(Pdi, PATTERN_VID).GetGroup(1);
                    var MatPID = Regex.Match(Pdi, PATTERN_PID).GetGroup(1);
                    var MatCOM = Regex.Match(Cap, PATTERN_COM).GetGroup(1);

                    if (string.IsNullOrWhiteSpace(MatVID) ||
                        string.IsNullOrWhiteSpace(MatPID) ||
                        string.IsNullOrWhiteSpace(MatCOM))
                        continue;

                    yield return new AcmDeviceInfo
                    {
                        PortName = MatCOM,
                        VendorId = MatVID.ToLower(),
                        ProductId = MatPID.ToLower(),
                        Caption = Cap,
                    };
                }
            }

            yield break;
        }
    }
}