namespace SimpleNP.Acm
{
    internal class InvokeOnDispose : IDisposable
    {
        private Action m_Action;

        /// <summary>
        /// Initialize a new <see cref="InvokeOnDispose"/>
        /// </summary>
        public InvokeOnDispose(Action Action)
        {
            m_Action = Action;
        }

        public void Dispose()
        {
            var Action = Interlocked.Exchange(ref m_Action, null);
            if (Action != null)
            {
                Action?.Invoke();
            }
        }
    }
}