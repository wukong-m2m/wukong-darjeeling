package javax.wukong.wkpf;

public abstract class VirtualWuObject {
    public void update() {} // TODO: this should be abstract, but nanovmtool chokes on abstract methods :-(

/* TODO: wanted to create these methods for convenience, but it seems inheritance doesn't fully work yet. nanovmtool gets into an endless loop loading subclasses while looking for these methods
    protected short getPropertyShort(byte propertyNumber) {
        return WKPF.getPropertyShort(this, propertyNumber);
    }
    protected void setPropertyShort(byte propertyNumber, short value) {
        WKPF.setPropertyShort(this, propertyNumber, value);
    }
    protected boolean getPropertyBoolean(byte propertyNumber) {
        return WKPF.getPropertyBoolean(this, propertyNumber);
    }
    protected void setPropertyBoolean(byte propertyNumber, boolean value) {
        WKPF.setPropertyBoolean(this, propertyNumber, value);
    }
    */
}
