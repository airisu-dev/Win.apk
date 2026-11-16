package android.content.om;

import android.os.Parcel;
import android.os.Parcelable;

/** API 34. */
public final class OverlayManagerTransaction implements Parcelable {
    private OverlayManagerTransaction() { }

    public static OverlayManagerTransaction newInstance() { return new OverlayManagerTransaction(); }
    public void registerFabricatedOverlay(FabricatedOverlay overlay) { }
    public void unregisterFabricatedOverlay(OverlayIdentifier overlay) { }

    @Override public int describeContents() { return 0; }
    @Override public void writeToParcel(Parcel dest, int flags) { }

    public static final Creator<OverlayManagerTransaction> CREATOR = new Creator<OverlayManagerTransaction>() {
        @Override public OverlayManagerTransaction createFromParcel(Parcel in) { return new OverlayManagerTransaction(); }
        @Override public OverlayManagerTransaction[] newArray(int size) { return new OverlayManagerTransaction[size]; }
    };
}
