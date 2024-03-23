namespace SimpleNP.Acm
{
    /// <summary>
    /// Recognition informations to scan ACM devices.
    /// </summary>
    public class AcmRecognitionInfo
    {
        /// <summary>
        /// Initialize a new <see cref="AcmRecognitionInfo"/>
        /// </summary>
        /// <param name="VendorId"></param>
        /// <param name="ProductId"></param>
        public AcmRecognitionInfo(string VendorId, string ProductId)
        {
            this.VendorId = VendorId;
            this.ProductId = ProductId;
        }

        /// <summary>
        /// Product Id.
        /// </summary>
        public string ProductId { get; }

        /// <summary>
        /// Vendor Id.
        /// </summary>
        public string VendorId { get; }

        // --

        /// <summary>
        /// White-list to scan ACM-NP control ports.
        /// </summary>
        internal static readonly AcmRecognitionInfo[] WHITELIST = new AcmRecognitionInfo[]
        {
             new("8857", "0323")
        };

        /// <summary>
        /// Number Pad, version: 2024-03-23.
        /// </summary>
        public static AcmRecognitionInfo NP_20240323 => WHITELIST[0];
    }
}