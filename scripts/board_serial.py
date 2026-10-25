"""USB Serial/JTAG connection that never issues DTR/RTS reset ioctls."""
import serial, termios
class BoardSerial(serial.Serial):
    def _update_dtr_state(self):
        pass
    def _update_rts_state(self):
        pass
    def open(self):
        super().open()
        # macOS must not drop modem lines when the diagnostic reader closes.
        attrs = termios.tcgetattr(self.fileno())
        attrs[2] &= ~termios.HUPCL
        attrs[2] |= termios.CLOCAL
        termios.tcsetattr(self.fileno(), termios.TCSANOW, attrs)
