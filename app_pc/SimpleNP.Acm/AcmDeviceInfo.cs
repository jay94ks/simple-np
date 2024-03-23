using System.Linq;

namespace SimpleNP.Acm
{
    /// <summary>
    /// Represents ACM device on the system installed.
    /// </summary>
    public class AcmDeviceInfo
    {
        private const StringComparison CMP_MODE = StringComparison.OrdinalIgnoreCase;

        /// <summary>
        /// Get all ACM port informations.
        /// </summary>
        /// <returns></returns>
        public static IEnumerable<AcmDeviceInfo> All() => AcmDeviceScanner.Scan();

        /// <summary>
        /// Port Name.
        /// </summary>
        public string PortName { get; set; }

        /// <summary>
        /// Product Id.
        /// </summary>
        public string ProductId { get; set; }

        /// <summary>
        /// Vendor Id.
        /// </summary>
        public string VendorId { get; set; }

        /// <summary>
        /// Caption string.
        /// </summary>
        public string Caption { get; set; }

        /// <summary>
        /// Test whether the USB's vendor and product id.
        /// </summary>
        /// <param name="VendorId"></param>
        /// <param name="ProductId"></param>
        /// <returns></returns>
        public bool Is(string VendorId, string ProductId)
        {
            var Pid = (this.ProductId ?? string.Empty).ToLower();
            var Vid = (this.VendorId ?? string.Empty).ToLower();

            if (Vid.Equals((VendorId ?? string.Empty).Trim().ToLower(), CMP_MODE) == false)
                return false;

            if (ProductId == "*")
                return true;

            if (Pid.Equals((ProductId ?? string.Empty).Trim().ToLower(), CMP_MODE) == false)
                return false;

            return true;
        }

        /// <summary>
        /// Test whether the USB is same with the given product or not.
        /// </summary>
        /// <param name="PortInfo"></param>
        /// <returns></returns>
        public bool Is(AcmRecognitionInfo PortInfo) => Is(PortInfo.VendorId, PortInfo.ProductId);

        // --
        private static readonly Dictionary<string, List<Action<string, bool>>> CALLBACKS = new();
        private static bool RUNNING = false;

        /// <summary>
        /// Monitor the specified port exists on system.
        /// </summary>
        /// <param name="PortName"></param>
        /// <param name="Callback"></param>
        /// <returns></returns>
        internal static IDisposable Monitor(string PortName, Action<string, bool> Callback)
        {
            lock (CALLBACKS)
            {
                if (CALLBACKS.TryGetValue(PortName, out var List) == false)
                    CALLBACKS[PortName] = List = new List<Action<string, bool>>();

                List.Add(Callback);
                if (RUNNING == false)
                {
                    RUNNING = true;
                    _ = RunMonitorLoop();
                }

                return new InvokeOnDispose(() =>
                {
                    lock (CALLBACKS)
                    {
                        if (CALLBACKS.TryGetValue(PortName, out var List) == false)
                            return;

                        List.Remove(Callback);
                    }
                });
            }
        }

        /// <summary>
        /// Run the monitor loop.
        /// </summary>
        /// <returns></returns>
        private static async Task RunMonitorLoop()
        {
            await Task.Yield();
            var Handled = new HashSet<string>();
            while (true)
            {
                await Task.Delay(100);
                lock (CALLBACKS)
                {
                    // --> if no callbacks registered, finish the task.
                    if (CALLBACKS.Count <= 0)
                    {
                        RUNNING = false;
                        break;
                    }
                }

                var Ports = All().Select(X => X.PortName).ToArray();
                var Inserts = Ports.Where(X => Handled.Contains(X) == false).ToArray();
                var Removals = Handled.Where(X => Ports.Contains(X) == false).ToArray();

                // --> remove all removed ports.
                Handled.RemoveWhere(Removals.Contains);

                // --> add all inserted ports.
                foreach (var Each in Inserts)
                    Handled.Add(Each);

                if (Inserts.Length <= 0 && Removals.Length <= 0)
                    continue;

                var Callbacks = new Dictionary<string, Action<string, bool>[]>();

                lock (CALLBACKS)
                {
                    if (CALLBACKS.Count <= 0)
                    {
                        RUNNING = false;
                        break;
                    }

                    foreach (var Each in Inserts.Concat(Removals))
                    {
                        if (CALLBACKS.TryGetValue(Each, out var List) == false)
                        {
                            Callbacks[Each] = Array.Empty<Action<string, bool>>();
                            continue;
                        }

                        Callbacks[Each] = List.ToArray();
                    }
                }

                foreach (var Each in Inserts)
                {
                    foreach (var Cb in Callbacks[Each])
                        Cb.Invoke(Each, true);
                }

                foreach (var Each in Removals)
                {
                    foreach (var Cb in Callbacks[Each])
                        Cb.Invoke(Each, false);
                }
            }
        }
    }
}