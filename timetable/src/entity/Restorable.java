package entity;

public interface Restorable {
    
    public final int ST_MASTER = 0;
    public final int ST_CANDIDATE = 1;
    public final int ST_BLANK = 2;
    
    public void takeSnapshot(int snapshotType);
    
    public void restoreSnapshot(int snapshotType);
    
}
