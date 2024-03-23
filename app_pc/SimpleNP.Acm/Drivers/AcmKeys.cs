namespace SimpleNP.Acm.Drivers
{
    /// <summary>
    /// Acm Keys.
    /// </summary>
    public enum AcmKeys
    {
        Ufn1 = 0,     // --> SW 2
        Slash = 1,     // --> SW 7
        Asteroid = 2,  // --> SW 12
        Minus = 3,     // --> SW 16
        Numlock = 4,   // --> SW 21

        Ufn2 = 5,     // --> SW 3
        Num7 = 6,     // --> SW 8
        Num8 = 7,     // --> SW 13
        Num9 = 8,     // --> SW 17
        Plus = 9,      // --> SW 22.

        Ufn3 = 10,    // --> SW 4
        Num4 = 11,    // --> SW 9
        Num5 = 12,    // --> SW 14
        Num6 = 13,    // --> SW 18
        Enter = 14,    // --> SW 23.

        Ufn4 = 15,    // --> SW 5
        Num1 = 16,    // --> SW 10
        Num2 = 17,    // --> SW 15
        Num3 = 18,    // --> SW 19

        Ufn5 = 20,    // --> SW 6
        Num0 = 21,    // --> SW 11
        Dot = 23,      // --> SW 20

        Mrec = 19,     // --> SW 24, GP13
        Mplay = 24,   // --> SW 25, GP22

        /* Maximum count of registrations of keys. */
        MAX = 25,
        INV = 0xff,
    };
}
