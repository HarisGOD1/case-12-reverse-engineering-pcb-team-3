// Import-time post-script: summarize the program, then decompile every function.
//@category Analysis

import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.mem.*;
import ghidra.program.model.address.*;
import java.io.*;

public class DecompileAll extends GhidraScript {
    @Override
    public void run() throws Exception {
        PrintWriter sum = new PrintWriter(new FileWriter("/tmp/opencode/program_summary.txt"));
        PrintWriter pw = new PrintWriter(new FileWriter("/tmp/opencode/program_decompiled.c"));

        sum.println("Program: " + currentProgram.getName());
        sum.println("Language: " + currentProgram.getLanguageID());
        sum.println("ImageBase: " + currentProgram.getImageBase());
        sum.println("Min: " + currentProgram.getMinAddress() + " Max: " + currentProgram.getMaxAddress());
        sum.println();
        sum.println("=== BLOCKS ===");
        for (MemoryBlock b : currentProgram.getMemory().getBlocks()) {
            sum.println(String.format("BLOCK|%s|%s-%s|size=0x%X|init=%s|R=%s W=%s X=%s",
                    b.getName(), b.getStart(), b.getEnd(), b.getSize(), b.isInitialized(),
                    b.isRead(), b.isWrite(), b.isExecute()));
        }

        // Function + string inventory
        FunctionManager fm = currentProgram.getFunctionManager();
        int fcount = 0;
        sum.println();
        sum.println("=== FUNCTIONS ===");
        for (Function f : fm.getFunctions(true)) {
            sum.println("FUNC|" + f.getEntryPoint() + "|" + f.getName() + "|" + f.getBody().getNumAddresses());
            fcount++;
        }
        sum.println("TOTAL_FUNCTIONS=" + fcount);

        Listing listing = currentProgram.getListing();
        DataIterator diter = listing.getDefinedData(true);
        int scount = 0;
        sum.println();
        sum.println("=== STRINGS ===");
        while (diter.hasNext() && !monitor.isCancelled()) {
            Data d = diter.next();
            if (d.hasStringValue()) {
                String v = d.getValue().toString();
                if (v.length() >= 3) {
                    sum.println("STR|" + d.getAddress() + "|" + v.replace("\n", "\\n").replace("\r", "\\r"));
                    scount++;
                }
            }
        }
        sum.println("TOTAL_STRINGS=" + scount);
        sum.close();

        // Decompile everything
        DecompInterface decomp = new DecompInterface();
        decomp.setOptions(new DecompileOptions());
        decomp.openProgram(currentProgram);

        pw.println("/* ================================================================ */");
        pw.println("/* Decompiled program: " + currentProgram.getName());
        pw.println("/* Language: " + currentProgram.getLanguageID() + "  ImageBase: " + currentProgram.getImageBase());
        pw.println("/* Functions: " + fcount);
        pw.println("/* ================================================================ */");
        pw.println();

        int ok = 0, fail = 0;
        for (Function f : fm.getFunctions(true)) {
            pw.println("/* ---------------------------------------------------------------- */");
            pw.println("/* " + f.getName() + " @ " + f.getEntryPoint() + "  (" + f.getBody().getNumAddresses() + " bytes) */");
            pw.println("/* ---------------------------------------------------------------- */");
            DecompileResults r = decomp.decompileFunction(f, 60, monitor);
            if (r != null && r.decompileCompleted() && r.getDecompiledFunction() != null) {
                pw.println(r.getDecompiledFunction().getC());
                ok++;
            } else {
                pw.println("/* <decompilation failed> */");
                fail++;
            }
            pw.println();
        }
        pw.println("/* TOTAL: " + fcount + " functions, decompiled ok=" + ok + " failed=" + fail + " */");
        decomp.dispose();
        pw.close();
        println("Decompiled ok=" + ok + " failed=" + fail + " -> /tmp/opencode/program_decompiled.c");
    }
}