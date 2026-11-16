package android.content.om;

import android.os.Parcel;
import android.os.Parcelable;

/** API 34. Never handed out: {@link OverlayManager} lists no overlays. */
public final class OverlayInfo implements Parcelable {
    private final String mPackageName;
    private final String mOverlayName;
    private final String mTargetPackageName;
    private final String mTargetOverlayableName;
    private final boolean mEnabled;
    private final boolean mFabricated;

    OverlayInfo(String packageName, String overlayName, String targetPackageName,
            String targetOverlayableName, boolean enabled, boolean fabricated) {
        mPackageName = packageName;
        mOverlayName = overlayName;
        mTargetPackageName = targetPackageName;
        mTargetOverlayableName = targetOverlayableName;
        mEnabled = enabled;
        mFabricated = fabricated;
    }

    public String getOverlayName() { return mOverlayName; }
    public String getTargetPackageName() { return mTargetPackageName; }
    public String getTargetOverlayableName() { return mTargetOverlayableName; }
    public boolean isEnabled() { return mEnabled; }
    public boolean isFabricated() { return mFabricated; }
    public OverlayIdentifier getOverlayIdentifier() { return new OverlayIdentifier(mPackageName, mOverlayName); }

    @Override public int describeContents() { return 0; }

    @Override public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(mPackageName);
        dest.writeString(mOverlayName);
        dest.writeString(mTargetPackageName);
        dest.writeString(mTargetOverlayableName);
        dest.writeInt(mEnabled ? 1 : 0);
        dest.writeInt(mFabricated ? 1 : 0);
    }

    public static final Creator<OverlayInfo> CREATOR = new Creator<OverlayInfo>() {
        @Override public OverlayInfo createFromParcel(Parcel in) {
            return new OverlayInfo(in.readString(), in.readString(), in.readString(), in.readString(),
                    in.readInt() != 0, in.readInt() != 0);
        }
        @Override public OverlayInfo[] newArray(int size) { return new OverlayInfo[size]; }
    };

    @Override public String toString() {
        return "OverlayInfo{" + getOverlayIdentifier() + " -> " + mTargetPackageName + "}";
    }
}
