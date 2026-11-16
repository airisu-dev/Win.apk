package android.content.om;

import java.util.Collections;
import java.util.List;

/** API 34. There is no overlay service: {@code getSystemService(OverlayManager.class)} is null,
 *  which is how apps learn overlays are unavailable. The class has to exist so that probe links. */
public final class OverlayManager {
    private OverlayManager() { }

    public void commit(OverlayManagerTransaction transaction) {
        throw new SecurityException("no overlay service");
    }

    public List<OverlayInfo> getOverlayInfosForTarget(String targetPackageName) {
        return Collections.emptyList();
    }
}
