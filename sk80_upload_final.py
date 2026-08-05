#!/usr/bin/env python3
"""
Upload GIFs/images to XVX S-K80 LCD display. 

Uses hidraw for BOTH control commands (HIDIOCSFEATURE) and data writes.

Protocol: 04 18 START -> 04 72 IMAGE_CFG -> DATA pages -> 04 02 SAVE
Internal res: 160x96 RGB565 LE, 256-byte header, 4096-byte pages

python3 sk80_upload_final.py /home/ubuntu/Downloads/29.gif
python3 sk80_upload_final.py --dry-run my-output ./samples/29.gif
"""
import argparse, sys, time, os, fcntl, select
from PIL import Image

VID, PID = 0x05ac, 0x024f
W, H = 160, 96
PAGE = 4096
MAX_FRAMES = 255
HIDIOCSFEATURE = 0xC0000000 | (65 << 16) | (ord('H') << 8) | 0x06
HIDIOCGFEATURE = 0xC0000000 | (65 << 16) | (ord('H') << 8) | 0x07

# ---------------------------------------------------------------------------
# Device discovery (real mode only)
# ---------------------------------------------------------------------------

def find_hidraw(usage_page_lo=None):
    """Find hidraw device by usage page low byte (e.g. 0x68 for 0xFF68)."""
    for h in sorted(os.listdir('/sys/class/hidraw/')):
        rdesc = f'/sys/class/hidraw/{h}/device/report_descriptor'
        if not os.path.exists(rdesc):
            continue
        data = open(rdesc, 'rb').read()
        uevent = open(f'/sys/class/hidraw/{h}/device/uevent').read()
        if '05AC' not in uevent or '024F' not in uevent:
            continue
        if usage_page_lo is not None:
            found = data.find(bytes([usage_page_lo, 0xFF]))
            if found >= 0:
                return f'/dev/{h}'
        else:
            return f'/dev/{h}'
    return None

def find_control_hidraw():
    """Find the control hidraw (the one that accepts HIDIOCSFEATURE)."""
    for h in sorted(os.listdir('/sys/class/hidraw/')):
        rdesc = f'/sys/class/hidraw/{h}/device/report_descriptor'
        if not os.path.exists(rdesc):
            continue
        uevent = open(f'/sys/class/hidraw/{h}/device/uevent').read()
        if '05AC' not in uevent or '024F' not in uevent:
            continue
        dev = f'/dev/{h}'
        try:
            fd = os.open(dev, os.O_RDWR)
            # Test with START command
            cmd = bytearray(65)
            cmd[0] = 0x00; cmd[1] = 0x04; cmd[2] = 0x18
            fcntl.ioctl(fd, HIDIOCSFEATURE, bytes(cmd))
            os.close(fd)
            return dev
        except OSError as exc:
            print(f"  Control probe failed for {dev}: {exc}")
            try:
                os.close(fd)
            except OSError:
                pass
    return None

# ---------------------------------------------------------------------------
# I/O abstraction
# ---------------------------------------------------------------------------

class RealIO:
    """Sends data to real hidraw devices."""

    def open(self):
        print("\n[0] Opening devices...")
        ctrl = find_control_hidraw()
        data_hidraw = find_hidraw(usage_page_lo=0x68)
        print(f"  Control: {ctrl}")
        print(f"  Data:    {data_hidraw}")
        if ctrl is None:
            raise RuntimeError(
                "Could not find a hidraw control interface that accepts "
                "HIDIOCSFEATURE. See control probe errors above."
            )
        if data_hidraw is None:
            raise RuntimeError(
                "Could not find the hidraw data interface with usage page 0xFF68."
            )
        self._ctrl_fd = os.open(ctrl, os.O_RDWR)
        self._data_fd = os.open(data_hidraw, os.O_RDWR)

    def send_feature(self, data_64, label=""):
        buf = bytearray(65)
        buf[0] = 0x00
        buf[1:65] = data_64[:64]
        fcntl.ioctl(self._ctrl_fd, HIDIOCSFEATURE, bytes(buf))

    def get_feature(self, label=""):
        buf = bytearray(65)
        buf[0] = 0x00
        fcntl.ioctl(self._ctrl_fd, HIDIOCGFEATURE, buf)
        return bytes(buf[1:])

    def write_data(self, data_4096, label=""):
        os.write(self._data_fd, b'\x00' + data_4096)

    def read_ack(self, timeout=0.3, label=""):
        r, _, _ = select.select([self._data_fd], [], [], timeout)
        if r:
            return os.read(self._data_fd, 64)
        return None

    def sleep(self, seconds, label=""):
        time.sleep(seconds)

    def close(self):
        os.close(self._ctrl_fd)
        os.close(self._data_fd)


class DryRunIO:
    """Captures all writes to a .bin file and timing to a .times file."""

    def __init__(self, prefix):
        self._prefix = prefix
        self._bin_buf = bytearray()
        self._times = []

    def open(self):
        print(f"\n[dry-run] Output will be written to:")
        print(f"  {self._prefix}.bin")
        print(f"  {self._prefix}.times")

        print("\n[dry-run] Discovering devices...")
        ctrl = find_control_hidraw()
        data_hidraw = find_hidraw(usage_page_lo=0x68)
        ctrl_str = ctrl or "not found"
        data_str = data_hidraw or "not found"
        print(f"  Control: {ctrl_str}")
        print(f"  Data:    {data_str}")

        self._times.append(f"# control={ctrl_str}  HIDIOCSFEATURE/HIDIOCGFEATURE (VID=05AC PID=024F)")
        self._times.append(f"# data={data_str}     usage_page=0xFF68 raw write/read (VID=05AC PID=024F)")
        self._times.append("#")
        self._times.append("# format: <interface> <operation> <label> <size_bytes>")
        self._times.append("#         sleep <duration_ms>ms")

    def send_feature(self, data_64, label=""):
        buf = bytearray(65)
        buf[0] = 0x00
        buf[1:65] = data_64[:64]
        self._bin_buf.extend(buf)
        self._times.append(f"control send_feature {label} 65")

    def get_feature(self, label=""):
        self._times.append(f"control get_feature {label} 65")
        return b'\x00' * 64

    def write_data(self, data_4096, label=""):
        buf = b'\x00' + bytes(data_4096)
        self._bin_buf.extend(buf)
        self._times.append(f"data write {label} {len(buf)}")

    def read_ack(self, timeout=0.3, label=""):
        self._times.append(f"data read_ack {label} 64")
        return b'\x00' * 4

    def sleep(self, seconds, label=""):
        ms = int(seconds * 1000)
        self._times.append(f"sleep {ms}ms")

    def close(self):
        bin_path = self._prefix + '.bin'
        times_path = self._prefix + '.times'
        with open(bin_path, 'wb') as f:
            f.write(self._bin_buf)
        with open(times_path, 'w') as f:
            for line in self._times:
                f.write(line + '\n')
        print(f"\n[dry-run] Wrote {len(self._bin_buf)} bytes to {bin_path}")
        print(f"[dry-run] Wrote {len(self._times)} events to {times_path}")

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="Upload GIFs/images to XVX S-K80 LCD display.")
    parser.add_argument('image', help="Path to image or GIF file")
    parser.add_argument('--dry-run', metavar='PREFIX',
                        help="Skip device I/O; write binary data to PREFIX.bin "
                             "and timing log to PREFIX.times")
    args = parser.parse_args()

    path = args.image
    dry_run = args.dry_run

    print("=" * 50)
    print("S-K80 Display Uploader" + (" [DRY RUN]" if dry_run else ""))
    print("=" * 50)
    
    # Load image
    img = Image.open(path)
    is_gif = path.lower().endswith('.gif') and getattr(img, 'n_frames', 1) > 1
    frames = min(getattr(img, 'n_frames', 1), MAX_FRAMES) if is_gif else 1
    print(f"\n{'GIF' if is_gif else 'Image'}: {frames} frame(s)")
    
    # Build header + pixel data
    hdr = bytearray(256)
    hdr[0] = frames
    all_px = bytearray()
    
    for i in range(frames):
        if is_gif:
            img.seek(i)
            ms = img.info.get('duration', 100)
            hdr[1+i] = max(1, ms // 2)
            # Convert directly - avoids PIL copy() bug with GIF disposal
            f = img.convert('RGBA')
            bg = Image.new('RGBA', f.size, (0,0,0,255))
            bg.paste(f, (0,0), f)
            f = bg.convert('RGB')
        else:
            f = img.convert('RGB')
            hdr[1] = 100
        
        f = f.resize((W, H), Image.LANCZOS)
        for y in range(H):
            for x in range(W):
                r, g, b = f.getpixel((x, y))
                v = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
                all_px.append(v & 0xFF)
                all_px.append((v >> 8) & 0xFF)
    
    full = bytes(hdr) + bytes(all_px)
    pages = [full[i:i+PAGE] for i in range(0, len(full), PAGE)]
    if len(pages[-1]) < PAGE:
        pages[-1] = pages[-1] + b'\xFF' * (PAGE - len(pages[-1]))
    total_pages = len(pages)
    print(f"Pages: {total_pages} ({len(full)} bytes)")
    
    # Set up I/O
    io = DryRunIO(dry_run) if dry_run else RealIO()
    io.open()
    
    # Step 1: START
    print("\n[1/4] START...")
    io.send_feature(bytes([0x04, 0x18]) + b'\x00' * 62, label="START")
    io.sleep(0.01, label="post-START")
    r = io.get_feature(label="START")
    print(f"  Handshake: {r[:8].hex(' ').upper()}")
    
    # Step 2: IMAGE_CFG
    print("[2/4] IMAGE_CFG...")
    cfg = bytearray(64)
    cfg[0] = 0x04; cfg[1] = 0x72; cfg[2] = frames
    cfg[8] = total_pages & 0xFF
    cfg[9] = (total_pages >> 8) & 0xFF
    io.send_feature(bytes(cfg), label="IMAGE_CFG")
    io.sleep(0.01, label="post-IMAGE_CFG")
    r = io.get_feature(label="IMAGE_CFG")
    print(f"  Handshake: {r[:8].hex(' ').upper()}")
    
    # Step 3: DATA pages
    print(f"[3/4] DATA ({total_pages} pages)...")
    for i, page in enumerate(pages):
        io.write_data(bytes(page), label=f"page[{i}]")
        io.sleep(0.005, label=f"post-page[{i}]")
        ack = io.read_ack(timeout=0.3, label=f"page[{i}]")
        ack_str = ack[:4].hex(' ').upper() if ack else 'NONE'
        if (i+1) % 20 == 0 or i == len(pages)-1:
            print(f"  {i+1}/{total_pages} ACK={ack_str}")
    
    # Wait
    print("  Waiting 3s...")
    io.sleep(3, label="pre-SAVE")
    
    # Step 4: SAVE
    print("[4/4] SAVE...")
    io.send_feature(bytes([0x04, 0x02]) + b'\x00' * 62, label="SAVE")
    io.sleep(0.01, label="post-SAVE")
    r = io.get_feature(label="SAVE")
    print(f"  Handshake: {r[:8].hex(' ').upper()}")
    
    io.close()
    print(f"\nDone! {frames} frame(s) {'captured' if dry_run else 'uploaded'}!")
    if not dry_run:
        print("Check your display!")

if __name__ == '__main__':
    main()
