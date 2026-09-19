// Dump non-default (renamed) function names to a file.
//@category Analysis

import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import java.io.*;

public class DumpNamed extends GhidraScript {
    @Override
    public void run() throws Exception {
        PrintWriter pw = new PrintWriter(new FileWriter("/tmp/opencode/named_functions.txt"));
        pw.println("Program: " + currentProgram.getName());
        pw.println("Language: " + currentProgram.getLanguageID());
        pw.println("ImageBase: " + currentProgram.getImageBase());
        pw.println();
        int total = 0, named = 0;
        for (Function f : currentProgram.getFunctionManager().getFunctions(true)) {
            total++;
            String n = f.getName();
            if (!n.startsWith("FUN_") && !n.startsWith("thunk_") && !n.startsWith("SUB_")) {
                pw.println("NAMED|" + f.getEntryPoint() + "|" + n + "|" + f.getBody().getNumAddresses() + " bytes");
                named++;
            }
        }
        pw.println();
        pw.println("TOTAL=" + total + " NAMED=" + named);
        pw.close();
        println("Named functions: " + named + " / " + total);
    }
}