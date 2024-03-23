using SimpleNP.Acm;
using SimpleNP.Acm.Drivers;

while (true)
{
    var Acms = AcmSettings.ScanAll().ToArray();
    foreach (var EachAcm in Acms)
    {
        Console.WriteLine($"Port: {EachAcm.Port}, VID: {EachAcm.RecognitionInfo.VendorId}, PID: {EachAcm.RecognitionInfo.ProductId}.");
    }

    Console.Write("Which port you want test? ");
    var Port = Console.ReadLine();

    if (string.IsNullOrWhiteSpace(Port) || Acms.Any(X => X.Port == Port) == false)
    {
        Console.WriteLine("Enter port name, please.");
        continue;
    }

    var Settings = Acms.FirstOrDefault(X => X.Port == Port);
    using var Acm = new AcmDriver(Settings);

    Acm.OnKeyStateChanged += Acm_OnKeyStateChanged;


    await Acm.PingAsync();

    try { await Task.Delay(Timeout.InfiniteTimeSpan, Acm.Disposing); }
    catch
    {
    }
}

void Acm_OnKeyStateChanged(AcmDriver Acm, AcmKeys Key, AcmKeyState State)
{
    Console.WriteLine($"{Key}: {State}.");
    Acm.PingAsync().ConfigureAwait(false).GetAwaiter().GetResult();
}