package org.ifaa.android.manager;

import android.app.Service;
import android.content.Intent;
import android.os.HwBinder;
import android.os.HwBlob;
import android.os.HwParcel;
import android.os.IBinder;
import android.os.IHwBinder;
import android.os.RemoteException;
import android.util.Slog;
import java.util.ArrayList;
import java.util.Arrays;

public class IFAAService extends Service {
    private static final boolean DEBUG = false;
    private static final String TAG = "IFAAService";

    private final IBinder mIFAABinder = new IIFAAService.Stub() {
        private static final String INTERFACE_DESCRIPTOR =
                "vendor.xiaomi.hardware.mlipay@1.0::IMlipayService";
        private static final String SERVICE_NAME =
                "vendor.xiaomi.hardware.mlipay@1.0::IMlipayService";

        private static final int CODE_PROCESS_CMD = 1;

        private IHwBinder mService;

        @Override
        public byte[] processCmd_v2(byte[] param) {
            HwParcel hwParcel = new HwParcel();
            try {
                if (mService == null) {
                    mService = HwBinder.getService(SERVICE_NAME, "default");
                }
                if (mService != null) {
                    HwParcel hwParcel2 = new HwParcel();
                    hwParcel2.writeInterfaceToken(INTERFACE_DESCRIPTOR);
                    ArrayList arrayList = new ArrayList(Arrays.asList(HwBlob.wrapArray(param)));
                    hwParcel2.writeInt8Vector(arrayList);
                    hwParcel2.writeInt32(arrayList.size());
                    mService.transact(CODE_PROCESS_CMD, hwParcel2, hwParcel, 0);
                    hwParcel.verifySuccess();
                    hwParcel2.releaseTemporaryStorage();
                    ArrayList readInt8Vector = hwParcel.readInt8Vector();
                    int size = readInt8Vector.size();
                    byte[] result = new byte[size];
                    for (int i = 0; i < size; i++) {
                        result[i] = ((Byte) readInt8Vector.get(i)).byteValue();
                    }
                    return result;
                }
            } catch (RemoteException e) {
                if (DEBUG) Slog.e(TAG, "transact failed. " + e);
            } finally {
                hwParcel.release();
            }
            if (DEBUG) Slog.e(TAG, "processCmdV2, return null");
            return null;
        }
    };

    @Override
    public IBinder onBind(Intent intent) {
        return mIFAABinder;
    }
}
