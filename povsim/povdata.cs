namespace povdata
{
    public class PovFramePayload
    {
        public byte[] leddata;

        public PovFramePayload()
        {
            leddata = new byte[Povgeometry.POV_FRAME_SIZE];
        }
    }
}