package android.content.om;

import android.content.res.AssetFileDescriptor;
import android.os.ParcelFileDescriptor;

/** API 34. Can be built, but {@link OverlayManager} cannot be had to commit it. */
public class FabricatedOverlay {
    private final String mOverlayName;
    private final String mTargetPackage;
    private String mTargetOverlayable;

    public FabricatedOverlay(String overlayName, String targetPackage) {
        if (overlayName == null || targetPackage == null) throw new NullPointerException();
        mOverlayName = overlayName;
        mTargetPackage = targetPackage;
    }

    public OverlayIdentifier getIdentifier() { return new OverlayIdentifier(null, mOverlayName); }
    public void setTargetOverlayable(String targetOverlayable) { mTargetOverlayable = targetOverlayable; }
    public void setResourceValue(String resourceName, int dataType, int value, String configuration) { }
    public void setResourceValue(String resourceName, String value, String configuration) { }
    public void setResourceValue(String resourceName, ParcelFileDescriptor value, String configuration) { }
    public void setResourceValue(String resourceName, AssetFileDescriptor value, String configuration) { }
}
