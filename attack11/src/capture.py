import os, glob, time, select, fcntl, struct, sys

TIOCMBIS = 0x5416
TIOCM_DTR, TIOCM_RTS = 0x002, 0x004
out_path = '/tmp/rp2350-otp/plan/dumper_capture.txt'
out = open(out_path, 'wb')
total = 0
last_open = 0.0
deadline = time.time() + 40

fd = None
while time.time() < deadline:
    if fd is None:
        for p in sorted(glob.glob('/dev/ttyACM*')):
            try:
                fd = os.open(p, os.O_RDONLY | os.O_NONBLOCK)
                fcntl.ioctl(fd, TIOCMBIS, struct.pack('I', TIOCM_DTR | TIOCM_RTS))
                last_open = time.time()
                sys.stderr.write('opened %s\n' % p)
                break
            except OSError:
                fd = None
        if fd is None:
            time.sleep(0.2)
            continue
    r, _, _ = select.select([fd], [], [], 0.5)
    if not r:
        if total and time.time() - last_open > 8:
            break
        continue
    try:
        chunk = os.read(fd, 1 << 16)
    except BlockingIOError:
        continue
    except OSError:
        chunk = b''
    if not chunk:
        try:
            os.close(fd)
        except OSError:
            pass
        fd = None
        time.sleep(0.5)
        if total and time.time() - last_open > 8:
            break
        continue
    out.write(chunk)
    out.flush()
    total += len(chunk)
    if total > 4_000_000:
        break

if fd is not None:
    os.close(fd)
out.close()
print('captured bytes:', total)
