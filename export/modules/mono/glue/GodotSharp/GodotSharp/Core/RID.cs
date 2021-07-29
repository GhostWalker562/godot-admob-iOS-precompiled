using System;
using System.Runtime.CompilerServices;

namespace Godot
{
    public sealed partial class RID : IDisposable
    {
        private bool disposed = false;

        internal IntPtr ptr;

        internal static IntPtr GetPtr(RID instance)
        {
            if (instance == null)
                throw new NullReferenceException($"The instance of type {nameof(RID)} is null.");

            if (instance.disposed)
                throw new ObjectDisposedException(instance.GetType().FullName);

            return instance.ptr;
        }

        ~RID()
        {
            Dispose(false);
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        private void Dispose(bool disposing)
        {
            if (disposed)
                return;

            if (ptr != IntPtr.Zero)
            {
                godot_icall_RID_Dtor(ptr);
                ptr = IntPtr.Zero;
            }

            disposed = true;
        }

        internal RID(IntPtr ptr)
        {
            this.ptr = ptr;
        }

        public IntPtr NativeInstance
        {
            get { return ptr; }
        }

        internal RID()
        {
            this.ptr = IntPtr.Zero;
        }

        public RID(Object from)
        {
            this.ptr = godot_icall_RID_Ctor(Object.GetPtr(from));
        }

        public int GetId()
        {
            return godot_icall_RID_get_id(RID.GetPtr(this));
        }

        public override string ToString() => "[RID]";

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static IntPtr godot_icall_RID_Ctor(IntPtr from);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void godot_icall_RID_Dtor(IntPtr ptr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static int godot_icall_RID_get_id(IntPtr ptr);
    }
}
