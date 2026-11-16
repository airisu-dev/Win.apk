package android.content.om;

import android.os.Parcel;
import android.os.Parcelable;
import java.util.Objects;

/** API 34. */
public final class OverlayIdentifier implements Parcelable {
    private final String mPackageName;
    private final String mOverlayName;

    OverlayIdentifier(String packageName, String overlayName) {
        mPackageName = packageName;
        mOverlayName = overlayName;
    }

    @Override public int describeContents() { return 0; }

    @Override public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(mPackageName);
        dest.writeString(mOverlayName);
    }

    public static final Creator<OverlayIdentifier> CREATOR = new Creator<OverlayIdentifier>() {
        @Override public OverlayIdentifier createFromParcel(Parcel in) {
            return new OverlayIdentifier(in.readString(), in.readString());
        }
        @Override public OverlayIdentifier[] newArray(int size) { return new OverlayIdentifier[size]; }
    };

    @Override public boolean equals(Object o) {
        if (!(o instanceof OverlayIdentifier)) return false;
        OverlayIdentifier other = (OverlayIdentifier) o;
        return Objects.equals(mPackageName, other.mPackageName) && Objects.equals(mOverlayName, other.mOverlayName);
    }

    @Override public int hashCode() { return Objects.hash(mPackageName, mOverlayName); }

    @Override public String toString() {
        return mOverlayName == null ? String.valueOf(mPackageName) : mPackageName + ":" + mOverlayName;
    }
}
