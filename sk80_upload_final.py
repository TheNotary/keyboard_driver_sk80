#!/usr/bin/env python3
"""
Upload GIFs/images to XVX S-K80 LCD display. 

Uses hidraw for BOTH control commands (HIDIOCSFEATURE) and data writes.

Protocol: 04 18 START -> 04 72 IMAGE_CFG -> DATA pages -> 04 02 SAVE
Internal res: 160x96 RGB565 LE, 256-byte header, 4096-byte pages

python3 sk80_upload_final.py /home/ubuntu/Downloads/29.gif
"""
import sys, time, os, fcntl, select
from PIL import Image

VID, PID = 0x05ac, 0x024f
W, H = 160, 96
PAGE = 4096
MAX_FRAMES = 255
HIDIOCSFEATURE = 0xC0000000 | (65 << 16) | (ord('H') << 8) | 0x06
HIDIOCGFEATURE = 0xC0000000 | (65 << 16) | (ord('H') << 8) | 0x07

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

def send_feature(fd, data_64):
    """Send 64-byte feature report via HIDIOCSFEATURE."""
    buf = bytearray(65)
    buf[0] = 0x00
    buf[1:65] = data_64[:64]
    fcntl.ioctl(fd, HIDIOCSFEATURE, bytes(buf))

def get_feature(fd):
    """Read feature report via HIDIOCGFEATURE."""
    buf = bytearray(65)
    buf[0] = 0x00
    fcntl.ioctl(fd, HIDIOCGFEATURE, buf)
    return bytes(buf[1:])

def write_data(fd, data_4096):
    """Write data page via hidraw (prepend 0x00 for kernel to strip)."""
    os.write(fd, b'\x00' + data_4096)

def read_ack(fd, timeout=0.3):
    """Read ACK from hidraw."""
    r, _, _ = select.select([fd], [], [], timeout)
    if r:
        return os.read(fd, 64)
    return None

def main():
    if len(sys.argv) < 2:
        print("Usage: sudo python3 sk80_upload_final.py <image>")
        sys.exit(1)
    
    path = sys.argv[1]
    print("=" * 50)
    print("S-K80 Display Uploader")
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
    
    # Find devices
    print("\n[0] Opening devices...")
    ctrl = find_control_hidraw()
    data_hidraw = find_hidraw(usage_page_lo=0x68)  # 0xFF68
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
    
    ctrl_fd = os.open(ctrl, os.O_RDWR)
    data_fd = os.open(data_hidraw, os.O_RDWR)
    
    # Step 1: START
    print("\n[1/4] START...")
    send_feature(ctrl_fd, bytes([0x04, 0x18]) + b'\x00' * 62)
    time.sleep(0.01)
    r = get_feature(ctrl_fd)
    print(f"  Handshake: {r[:8].hex(' ').upper()}")
    
    # Step 2: IMAGE_CFG
    print("[2/4] IMAGE_CFG...")
    cfg = bytearray(64)
    cfg[0] = 0x04; cfg[1] = 0x72; cfg[2] = frames
    cfg[8] = total_pages & 0xFF
    cfg[9] = (total_pages >> 8) & 0xFF
    send_feature(ctrl_fd, bytes(cfg))
    time.sleep(0.01)
    r = get_feature(ctrl_fd)
    print(f"  Handshake: {r[:8].hex(' ').upper()}")
    
    # Step 3: DATA pages
    print(f"[3/4] DATA ({total_pages} pages)...")
    for i, page in enumerate(pages):
        write_data(data_fd, bytes(page))
        time.sleep(0.005)
        ack = read_ack(data_fd, timeout=0.3)
        ack_str = ack[:4].hex(' ').upper() if ack else 'NONE'
        if (i+1) % 20 == 0 or i == len(pages)-1:
            print(f"  {i+1}/{total_pages} ACK={ack_str}")
    
    # Wait
    print("  Waiting 3s...")
    time.sleep(3)
    
    # Step 4: SAVE
    print("[4/4] SAVE...")
    send_feature(ctrl_fd, bytes([0x04, 0x02]) + b'\x00' * 62)
    time.sleep(0.01)
    r = get_feature(ctrl_fd)
    print(f"  Handshake: {r[:8].hex(' ').upper()}")
    
    os.close(ctrl_fd)
    os.close(data_fd)
    print(f"\nDone! {frames} frame(s) uploaded!")
    print("Check your display!")

if __name__ == '__main__':
    main()
