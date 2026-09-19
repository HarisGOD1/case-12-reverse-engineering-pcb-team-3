// Dump the memory map of the RP2040 flash dump and export the full raw image.
//@category Analysis

import ghidra.app.script.GhidraScript;
import ghidra.program.model.mem.*;
import ghidra.program.model.address.*;
import java.io.*;

public class DumpMemoryMap extends GhidraScript {
    @Override
    public void run() throws Exception {
        PrintWriter pw = new PrintWriter(new FileWriter("/tmp/opencode/memmap.txt"));
        Memory mem = currentProgram.getMemory();

        pw.println("Program: " + currentProgram.getName());
        pw.println("Language: " + currentProgram.getLanguageID());
        pw.println("ImageBase: " + currentProgram.getImageBase());
        pw.println("MinAddress: " + currentProgram.getMinAddress());
        pw.println("MaxAddress: " + currentProgram.getMaxAddress());
        pw.println();

        pw.println("=== MEMORY BLOCKS ===");
        long totalInit = 0;
        for (MemoryBlock b : mem.getBlocks()) {
            pw.println(String.format("BLOCK|%s|start=%s|end=%s|size=0x%X|init=%s|R=%s W=%s X=%s|volatile=%s|type=%s",
                    b.getName(), b.getStart(), b.getEnd(), b.getSize(), b.isInitialized(),
                    b.isRead(), b.isWrite(), b.isExecute(), b.isVolatile(), b.getType()));
            if (b.isInitialized()) {
                totalInit += b.getSize();
            }
        }
        pw.println("TOTAL_INITIALIZED_BYTES=0x" + Long.toHexString(totalInit));
        pw.println();

        // Export the whole initialized image flat, keyed by address offset from the
        // lowest initialized block start.
        Address minAddr = null;
        Address maxAddr = null;
        for (MemoryBlock b : mem.getBlocks()) {
            if (!b.isInitialized()) continue;
            if (minAddr == null || b.getStart().compareTo(minAddr) < 0) minAddr = b.getStart();
            if (maxAddr == null || b.getEnd().compareTo(maxAddr) > 0) maxAddr = b.getEnd();
        }
        pw.println("EXPORT minAddr=" + minAddr + " maxAddr=" + maxAddr);

        long span = maxAddr.subtract(minAddr) + 1;
        byte[] image = new byte[(int) span];
        java.util.Arrays.fill(image, (byte) 0xFF); // erased flash default

        for (MemoryBlock b : mem.getBlocks()) {
            if (!b.isInitialized()) continue;
            Address a = b.getStart();
            long off = a.subtract(minAddr);
            long len = b.getSize();
            byte[] buf = new byte[(int) len];
            int got = mem.getBytes(a, buf);
            for (int i = 0; i < got; i++) {
                image[(int) (off + i)] = buf[i];
            }
            pw.println(String.format("COPIED|%s|off=0x%X|len=0x%X|got=0x%X", b.getName(), off, len, got));
        }

        try (FileOutputStream fos = new FileOutputStream("/tmp/opencode/memory_full.bin")) {
            fos.write(image);
        }
        pw.println("WROTE /tmp/opencode/memory_full.bin size=0x" + Long.toHexString(span));

        pw.close();
        println("DumpMemoryMap done");
    }
}