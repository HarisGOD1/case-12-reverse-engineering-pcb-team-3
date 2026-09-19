/* ================================================================ */
/* Decompiled program: program.bin
/* Language: ARM:LE:32:Cortex  ImageBase: 00000000
/* Functions: 155
/* ================================================================ */

/* ---------------------------------------------------------------- */
/* FUN_10000000 @ 10000000  (238 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10000000(void)

{
  bool bVar1;
  uint uVar2;
  int iVar3;
  undefined4 uVar4;
  undefined4 extraout_r2;
  undefined4 *extraout_r3;
  int extraout_r3_00;
  int extraout_r3_01;
  undefined4 *extraout_r3_02;
  undefined4 *puVar5;
  int extraout_r3_03;
  int unaff_lr;
  
  iVar3 = DAT_100000cc;
  *(undefined4 *)(DAT_100000cc + 4) = 0x21;
  uVar2 = *(uint *)(iVar3 + 8) & 0xfffffffd;
  *(uint *)(iVar3 + 8) = uVar2;
  *(uint *)(iVar3 + 0xc) = uVar2;
  *(uint *)(iVar3 + 0x10) = uVar2;
  *(uint *)(iVar3 + 0x14) = uVar2;
  puVar5 = DAT_100000d0;
  DAT_100000d0[2] = 0;
  puVar5[5] = 2;
  puVar5[0x3c] = 1;
  *puVar5 = DAT_100000d4;
  puVar5[2] = 1;
  iVar3 = FUN_100000bc(0x35);
  puVar5 = extraout_r3;
  if (iVar3 != 2) {
    extraout_r3[0x18] = 6;
    FUN_100000aa();
    *(undefined4 *)(extraout_r3_00 + 0x60) = 1;
    *(undefined4 *)(extraout_r3_00 + 0x60) = 0;
    *(undefined4 *)(extraout_r3_00 + 0x60) = extraout_r2;
    FUN_100000aa();
    uVar4 = *(undefined4 *)(extraout_r3_01 + 0x60);
    do {
      uVar2 = FUN_100000bc(5,uVar4);
      uVar4 = 1;
      puVar5 = extraout_r3_02;
    } while ((uVar2 & 1) != 0);
  }
  puVar5[2] = 0;
  *puVar5 = DAT_100000d8;
  puVar5[1] = 0;
  *DAT_100000e0 = DAT_100000dc;
  puVar5[2] = 1;
  puVar5[0x18] = 0xeb;
  puVar5[0x18] = 0xa0;
  FUN_100000aa();
  *(undefined4 *)(extraout_r3_03 + 8) = 0;
  *DAT_100000e0 = DAT_100000e4;
  *(undefined4 *)(extraout_r3_03 + 8) = 1;
  puVar5 = DAT_100000e8;
  if (unaff_lr != 0) {
    return;
  }
  *DAT_100000ec = (int)DAT_100000e8;
  uVar4 = *puVar5;
  bVar1 = (bool)isCurrentModePrivileged();
  if (bVar1) {
    setMainStackPointer(uVar4);
  }
                    /* WARNING: Could not recover jumptable at 0x100000a8. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)puVar5[1])(uVar4);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_100000aa @ 100000aa  (18 bytes) */
/* ---------------------------------------------------------------- */

undefined8 FUN_100000aa(undefined4 param_1,undefined4 param_2,undefined4 param_3,int param_4)

{
  do {
    do {
    } while ((*(uint *)(param_4 + 0x28) & 4) == 0);
  } while ((*(uint *)(param_4 + 0x28) & 1) != 0);
  return CONCAT44(param_2,param_1);
}



/* ---------------------------------------------------------------- */
/* FUN_100000bc @ 100000bc  (16 bytes) */
/* ---------------------------------------------------------------- */

undefined8 FUN_100000bc(undefined4 param_1,undefined4 param_2,undefined4 param_3,int param_4)

{
  int extraout_r3;
  
  *(undefined4 *)(param_4 + 0x60) = param_1;
  *(undefined4 *)(param_4 + 0x60) = param_1;
  FUN_100000aa();
  return CONCAT44(param_2,*(undefined4 *)(extraout_r3 + 0x60));
}



/* ---------------------------------------------------------------- */
/* FUN_10000232 @ 10000232  (10 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10000232(undefined4 param_1,undefined4 *param_2,undefined4 *param_3,undefined4 *param_4)

{
  undefined4 uVar1;
  
  for (; param_3 < param_4; param_3 = param_3 + 1) {
    uVar1 = *param_2;
    param_2 = param_2 + 1;
    *param_3 = uVar1;
  }
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10000288 @ 10000288  (28 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10000288(void)

{
  if (((DAT_100002a8 - DAT_100002a4 >> 2) - (DAT_100002a8 - DAT_100002a4 >> 0x1f) >> 1 != 0) &&
     (DAT_100002ac != (code *)0x0)) {
    (*DAT_100002ac)();
  }
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_100002b0 @ 100002b0  (20 bytes) */
/* ---------------------------------------------------------------- */

void FUN_100002b0(undefined4 param_1,undefined4 param_2)

{
  if (DAT_100002c8 != 0) {
    param_1 = DAT_100002d0;
    param_2 = DAT_100002cc;
  }
  FUN_10000288(param_1,param_2);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_100002d4 @ 100002d4  (2 bytes) */
/* ---------------------------------------------------------------- */

void FUN_100002d4(void)

{
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_100002d8 @ 100002d8  (2 bytes) */
/* ---------------------------------------------------------------- */

void FUN_100002d8(void)

{
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_100002dc @ 100002dc  (2 bytes) */
/* ---------------------------------------------------------------- */

void FUN_100002dc(void)

{
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_100002e0 @ 100002e0  (2 bytes) */
/* ---------------------------------------------------------------- */

void FUN_100002e0(void)

{
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_100002e8 @ 100002e8  (76 bytes) */
/* ---------------------------------------------------------------- */

void FUN_100002e8(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  undefined4 local_34;
  undefined4 local_30;
  undefined4 uStack_2c;
  undefined4 local_28;
  undefined4 uStack_24;
  undefined4 uStack_20;
  undefined4 local_1c;
  
  local_30 = *DAT_10000334;
  uStack_2c = DAT_10000334[1];
  local_28 = *DAT_10000338;
  uStack_24 = DAT_10000338[1];
  uStack_20 = DAT_10000338[2];
  local_1c = DAT_10000338[3];
  local_34 = DAT_1000033c;
  FUN_10002e1c(param_2,&local_30,8);
  FUN_10002e1c(param_3,&local_28,0x10);
  FUN_10002e1c(param_4,&local_34,4);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10000350 @ 10000350  (4 bytes) */
/* ---------------------------------------------------------------- */

undefined4 FUN_10000350(void)

{
  return 1;
}



/* ---------------------------------------------------------------- */
/* FUN_10000354 @ 10000354  (408 bytes) */
/* ---------------------------------------------------------------- */

undefined4 FUN_10000354(undefined4 param_1,uint param_2,uint param_3,undefined4 param_4)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  int iVar5;
  int iVar6;
  uint *puVar7;
  int *piVar8;
  int *piVar9;
  
  iVar1 = DAT_100004ec;
  piVar8 = (int *)(&stack0xffffffdc + DAT_100004ec);
  piVar9 = (int *)(&stack0xffffffdc + DAT_100004ec);
  *(undefined4 *)(&stack0xffffffdc + DAT_100004ec) = param_4;
  if ((param_3 < 0x200) && (param_2 < 0x800)) {
    uVar3 = (uint)CARRY4(param_3,*(uint *)(&stack0x0000020c + DAT_100004ec));
    uVar2 = 0x800 - param_2 >> 0x17;
    if ((uVar3 <= uVar2) &&
       ((uVar3 != uVar2 ||
        (param_3 + *(uint *)(&stack0x0000020c + DAT_100004ec) <= (0x800 - param_2) * 0x200)))) {
      if (*(int *)(&stack0x0000020c + DAT_100004ec) != 0) {
        uVar3 = param_2 * DAT_100004f0 + DAT_100004f4;
        iVar6 = param_2 << 5;
        uVar2 = *(uint *)(&stack0x0000020c + DAT_100004ec);
        *(undefined1 **)(&stack0xffffffe0 + DAT_100004ec) = &stack0xffffffe4 + DAT_100004ec;
        do {
          FUN_10002e1c(*(undefined4 *)(&stack0xffffffe0 + iVar1),iVar6 * 0x10 + DAT_100004f8,0x200);
          puVar7 = (uint *)(&stack0xffffffe4 + iVar1);
          *(uint **)(&stack0xffffffe0 + iVar1) = puVar7;
          uVar4 = uVar3;
          do {
            puVar7[1] = (uVar4 + DAT_100004fc >> 0x18 | (uVar4 + DAT_10000500 >> 0x18) << 8 |
                         (uVar4 + DAT_10000504 >> 0x18) << 0x10 | uVar4 + DAT_10000508 & 0xff000000)
                        ^ puVar7[1];
            puVar7[2] = (uVar4 + DAT_1000050c >> 0x18 | (uVar4 + DAT_10000510 >> 0x18) << 8 |
                         (uVar4 + DAT_10000514 >> 0x18) << 0x10 | uVar4 + DAT_10000518 & 0xff000000)
                        ^ puVar7[2];
            puVar7[3] = (uVar4 + DAT_10000520 >> 0x18 | (uVar4 + DAT_1000051c >> 0x18) << 8 |
                         (uVar4 + DAT_10000524 >> 0x18) << 0x10 | uVar4 + DAT_10000528 & 0xff000000)
                        ^ puVar7[3];
            *puVar7 = *puVar7 ^ (uVar4 + DAT_1000052c & 0xff000000 |
                                (uVar4 + DAT_10000538 >> 0x18) << 0x10 |
                                (uVar4 >> 0x18) << 8 | uVar4 + DAT_10000534 >> 0x18);
            puVar7 = puVar7 + 4;
            uVar4 = uVar4 + DAT_10000530;
          } while (puVar7 != (uint *)(&stack0x000001e4 + iVar1));
          uVar4 = 0x200 - param_3;
          if (uVar2 < 0x200 - param_3) {
            uVar4 = uVar2;
          }
          iVar5 = *piVar8;
          FUN_10002e1c(iVar5,&stack0xffffffe4 + param_3 + iVar1,uVar4);
          *piVar9 = iVar5 + uVar4;
          uVar3 = uVar3 + DAT_100004f0;
          param_3 = 0;
          uVar2 = uVar2 - uVar4;
          iVar6 = iVar6 + 0x20;
        } while (uVar2 != 0);
      }
      return *(undefined4 *)(&stack0x0000020c + iVar1);
    }
  }
  return 0xffffffff;
}



/* ---------------------------------------------------------------- */
/* FUN_1000053c @ 1000053c  (764 bytes) */
/* ---------------------------------------------------------------- */

undefined4 FUN_1000053c(undefined4 param_1,uint param_2,uint param_3,undefined4 param_4)

{
  bool bVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  uint *puVar5;
  int iVar6;
  int iVar7;
  uint uVar8;
  uint uVar9;
  uint *puVar10;
  uint uVar11;
  uint uVar12;
  uint uVar13;
  
  iVar3 = DAT_10000844;
  iVar2 = DAT_10000838;
  *(undefined4 *)(&stack0xffffffec + DAT_10000838) = param_4;
  if ((param_3 < 0x200) && (param_2 < 0x800)) {
    uVar9 = (uint)CARRY4(*(uint *)(&stack0x00000424 + DAT_10000838),param_3);
    uVar4 = 0x800 - param_2 >> 0x17;
    if ((uVar9 <= uVar4) &&
       ((uVar9 != uVar4 ||
        (*(uint *)(&stack0x00000424 + DAT_10000838) + param_3 <= (0x800 - param_2) * 0x200)))) {
      uVar9 = 0;
      bVar1 = (bool)isCurrentModePrivileged();
      if (bVar1) {
        uVar9 = isIRQinterruptsEnabled();
      }
      disableIRQinterrupts();
      if (*(int *)(&stack0x00000424 + DAT_10000838) != 0) {
        *(uint *)(&stack0xffffffe4 + DAT_10000838) = param_2 * DAT_10000840 + DAT_1000083c;
        *(uint *)(&stack0xffffffe8 + DAT_10000838) = param_2 << 5;
        iVar7 = *(int *)(&stack0x00000424 + DAT_10000838);
        *(undefined1 **)(&stack0xfffffff0 + DAT_10000838) = &stack0xfffffffc + DAT_10000838;
        *(undefined1 **)(&stack0xffffffe0 + DAT_10000838) = &stack0x000001fc + DAT_10000838;
        while( true ) {
          iVar6 = *(int *)(&stack0xffffffe8 + iVar2);
          FUN_10002e1c(*(undefined4 *)(&stack0xfffffff0 + iVar2),DAT_10000848 + iVar6 * 0x10,0x200);
          *(uint *)(&stack0xfffffff8 + iVar2) = param_3;
          puVar5 = *(uint **)(&stack0xffffffe0 + iVar2);
          puVar10 = (uint *)(&stack0xfffffffc + iVar2);
          *(uint **)(&stack0xfffffff0 + iVar2) = puVar10;
          uVar4 = *(uint *)(&stack0xffffffe4 + iVar2);
          *(int *)(&stack0xfffffff4 + iVar2) = iVar7;
          do {
            puVar10[1] = (uVar4 + DAT_1000084c >> 0x18 | (uVar4 + DAT_10000850 >> 0x18) << 8 |
                          (uVar4 + DAT_10000854 >> 0x18) << 0x10 | uVar4 + DAT_10000858 & 0xff000000
                         ) ^ puVar10[1];
            puVar10[2] = (uVar4 + DAT_1000085c >> 0x18 | (uVar4 + DAT_10000860 >> 0x18) << 8 |
                          (uVar4 + DAT_10000864 >> 0x18) << 0x10 | uVar4 + DAT_10000868 & 0xff000000
                         ) ^ puVar10[2];
            uVar11 = uVar4 >> 0x18;
            puVar10[3] = ((uVar4 + DAT_10000870 >> 0x18) << 8 | uVar4 + DAT_1000086c >> 0x18 |
                          (uVar4 + DAT_10000874 >> 0x18) << 0x10 | uVar4 + DAT_10000878 & 0xff000000
                         ) ^ puVar10[3];
            uVar8 = uVar4 + DAT_10000880;
            uVar13 = uVar4 + DAT_10000884;
            uVar12 = uVar4 + DAT_10000888;
            uVar4 = uVar4 + DAT_1000087c;
            *puVar10 = (uVar8 >> 0x18 | uVar11 << 8 | (uVar13 >> 0x18) << 0x10 | uVar12 & 0xff000000
                       ) ^ *puVar10;
            puVar10 = puVar10 + 4;
          } while (puVar5 != puVar10);
          uVar8 = *(uint *)(&stack0xfffffff4 + iVar2);
          uVar4 = 0x200 - *(int *)(&stack0xfffffff8 + iVar2);
          if (uVar8 < uVar4) {
            uVar4 = uVar8;
          }
          iVar7 = *(int *)(&stack0xfffffff0 + iVar2);
          FUN_10002e1c(iVar7 + *(int *)(&stack0xfffffff8 + iVar2),
                       *(undefined4 *)(&stack0xffffffec + iVar2),uVar4);
          FUN_10002e1c(*(undefined4 *)(&stack0xffffffe0 + iVar2),iVar7,0x200);
          puVar10 = (uint *)(&stack0x000001fc + iVar2);
          *(uint **)(&stack0xffffffe0 + iVar2) = puVar10;
          uVar11 = *(uint *)(&stack0xffffffe4 + iVar2);
          *(uint *)(&stack0xfffffff4 + iVar2) = uVar4;
          do {
            puVar10[1] = (uVar11 + DAT_1000084c >> 0x18 | (uVar11 + DAT_10000850 >> 0x18) << 8 |
                          (uVar11 + DAT_10000854 >> 0x18) << 0x10 |
                         uVar11 + DAT_10000858 & 0xff000000) ^ puVar10[1];
            puVar10[2] = (uVar11 + DAT_1000085c >> 0x18 | (uVar11 + DAT_10000860 >> 0x18) << 8 |
                          (uVar11 + DAT_10000864 >> 0x18) << 0x10 |
                         uVar11 + DAT_10000868 & 0xff000000) ^ puVar10[2];
            puVar10[3] = (uVar11 + DAT_1000086c >> 0x18 | (uVar11 + DAT_10000870 >> 0x18) << 8 |
                          (uVar11 + DAT_10000874 >> 0x18) << 0x10 |
                         uVar11 + DAT_10000878 & 0xff000000) ^ puVar10[3];
            *puVar10 = (uVar11 + DAT_10000880 >> 0x18 | (uVar11 >> 0x18) << 8 |
                        (uVar11 + DAT_10000884 >> 0x18) << 0x10 | uVar11 + DAT_10000888 & 0xff000000
                       ) ^ *puVar10;
            puVar10 = puVar10 + 4;
            uVar11 = uVar11 + DAT_1000087c;
          } while (puVar10 != (uint *)(&stack0x000003fc + iVar2));
          uVar11 = iVar6 * 0x10 + 0x100000;
          iVar7 = 0;
          uVar4 = 0x200;
          do {
            uVar13 = uVar11 & 0xfffff000;
            uVar12 = (uVar13 - uVar11) + 0x1000;
            if (uVar4 < uVar12) {
              uVar12 = uVar4;
            }
            FUN_10002e1c(iVar3,uVar13 + 0x10000000,0x1000);
            FUN_10002e1c((uVar11 - uVar13) + iVar3,*(int *)(&stack0xffffffe0 + iVar2) + iVar7,uVar12
                        );
            FUN_10005298(uVar13,0x1000);
            FUN_10005278(uVar13,iVar3,0x1000);
            uVar11 = uVar11 + uVar12;
            iVar7 = iVar7 + uVar12;
            uVar4 = uVar4 - uVar12;
          } while (uVar4 != 0);
          *(int *)(&stack0xffffffec + iVar2) =
               *(int *)(&stack0xffffffec + iVar2) + *(int *)(&stack0xfffffff4 + iVar2);
          iVar7 = uVar8 - *(int *)(&stack0xfffffff4 + iVar2);
          *(int *)(&stack0xffffffe4 + iVar2) = *(int *)(&stack0xffffffe4 + iVar2) + DAT_10000840;
          *(int *)(&stack0xffffffe8 + iVar2) = *(int *)(&stack0xffffffe8 + iVar2) + 0x20;
          if (iVar7 == 0) break;
          param_3 = 0;
        }
      }
      bVar1 = (bool)isCurrentModePrivileged();
      if (bVar1) {
        enableIRQinterrupts((uVar9 & 1) == 1);
      }
      return *(undefined4 *)(&stack0x00000424 + iVar2);
    }
  }
  return 0xffffffff;
}



/* ---------------------------------------------------------------- */
/* FUN_10000890 @ 10000890  (2 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10000890(void)

{
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10000894 @ 10000894  (6 bytes) */
/* ---------------------------------------------------------------- */

undefined4 FUN_10000894(void)

{
  return 0xffffffff;
}



/* ---------------------------------------------------------------- */
/* FUN_1000089c @ 1000089c  (38 bytes) */
/* ---------------------------------------------------------------- */

void FUN_1000089c(void)

{
  FUN_100050e0();
  FUN_1000305c();
  FUN_10000ab0();
  FUN_10004e64(0,0);
  do {
    FUN_10003858(0xffffffff,0);
    FUN_100009a4();
  } while( true );
}



/* ---------------------------------------------------------------- */
/* FUN_100008c4 @ 100008c4  (4 bytes) */
/* ---------------------------------------------------------------- */

undefined4 FUN_100008c4(void)

{
  return DAT_100008c8;
}



/* ---------------------------------------------------------------- */
/* FUN_100008cc @ 100008cc  (4 bytes) */
/* ---------------------------------------------------------------- */

undefined4 FUN_100008cc(void)

{
  return DAT_100008d0;
}



/* ---------------------------------------------------------------- */
/* FUN_100008d4 @ 100008d4  (100 bytes) */
/* ---------------------------------------------------------------- */

ushort * FUN_100008d4(uint param_1)

{
  byte *pbVar1;
  byte bVar2;
  ushort *puVar3;
  ushort uVar4;
  uint uVar5;
  byte bVar6;
  int iVar7;
  
  puVar3 = DAT_1000093c;
  if (param_1 == 0) {
    DAT_1000093c[1] = *(ushort *)*DAT_10000938;
    uVar4 = 0x304;
  }
  else {
    if (4 < param_1) {
      return (ushort *)0x0;
    }
    iVar7 = DAT_10000938[param_1];
    bVar2 = FUN_100051c8(iVar7);
    bVar6 = bVar2;
    if (0x1f < bVar2) {
      bVar6 = 0x1f;
    }
    if (bVar2 != 0) {
      uVar5 = 0;
      puVar3 = DAT_10000940;
      do {
        pbVar1 = (byte *)(iVar7 + uVar5);
        uVar5 = uVar5 + 1;
        *puVar3 = (ushort)*pbVar1;
        puVar3 = puVar3 + 1;
      } while ((uVar5 & 0xff) < (uint)bVar6);
    }
    uVar4 = (ushort)((bVar6 + 1 & 0xff) << 1) | 0x300;
    puVar3 = DAT_1000093c;
  }
  *puVar3 = uVar4;
  return puVar3;
}



/* ---------------------------------------------------------------- */
/* FUN_100009a4 @ 100009a4  (240 bytes) */
/* ---------------------------------------------------------------- */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_100009a4(void)

{
  uint uVar1;
  char *pcVar2;
  uint *puVar3;
  uint uVar4;
  longlong lVar5;
  undefined8 uVar6;
  
  pcVar2 = DAT_10000a98;
  if (*DAT_10000a94 == '\0') {
    *DAT_10000a98 = '\0';
  }
  else if (*DAT_10000a98 == '\0') {
    _DAT_d0000018 = 0x100;
    *DAT_10000a9c = 0;
    lVar5 = FUN_100025c8();
    lVar5 = lVar5 + (ulonglong)DAT_10000aa0;
    if (lVar5 < 0) {
      lVar5 = CONCAT44(DAT_10000aa8,0xffffffff);
    }
    *(longlong *)DAT_10000aa4 = lVar5;
    *pcVar2 = '\x01';
  }
  else {
    uVar6 = FUN_100025c8();
    puVar3 = DAT_10000aa4;
    uVar4 = DAT_10000aa4[1] - (int)((ulonglong)uVar6 >> 0x20);
    uVar1 = (uint)(*DAT_10000aa4 < (uint)uVar6);
    if (((int)(uVar4 - uVar1) < 1) && ((uVar4 != uVar1 || (*DAT_10000aa4 == (uint)uVar6)))) {
      lVar5 = FUN_100025c8();
      lVar5 = lVar5 + (ulonglong)DAT_10000aa0;
      if (lVar5 < 0) {
        lVar5 = CONCAT44(DAT_10000aa8,0xffffffff);
      }
      *(longlong *)puVar3 = lVar5;
      _DAT_d0000018 = 0x100;
      uVar4 = *DAT_10000a9c + 1;
      _DAT_d0000014 = 1 << *(sbyte *)(DAT_10000aac + *DAT_10000a9c);
      if (uVar4 < 10) {
        *DAT_10000a9c = uVar4;
      }
      else {
        *DAT_10000a9c = 0;
      }
    }
  }
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10000ab0 @ 10000ab0  (1558 bytes) */
/* ---------------------------------------------------------------- */

/* WARNING: Removing unreachable block (ram,0x10000e76) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_10000ab0(void)

{
  byte bVar1;
  char cVar2;
  uint uVar3;
  bool bVar4;
  bool bVar5;
  bool bVar6;
  bool bVar7;
  char *pcVar8;
  byte *pbVar9;
  byte *pbVar10;
  char *pcVar11;
  char *pcVar12;
  int iVar13;
  byte *pbVar14;
  int iVar15;
  uint uVar16;
  byte *pbVar17;
  uint uVar18;
  longlong lVar19;
  longlong lVar20;
  longlong lVar21;
  undefined8 uVar22;
  longlong lVar23;
  int local_54;
  uint local_50;
  int local_4c;
  uint local_48;
  int local_44;
  int local_3c;
  char cStack_2d;
  int local_2c;
  
  FUN_100016b8(0x1d);
  _DAT_d0000028 = 0x20000000;
  FUN_10001568(0x1d,1,0);
  FUN_100016b8(0x1b);
  _DAT_d0000028 = 0x8000000;
  FUN_10001568(0x1b,1,0);
  FUN_100016b8(0x1c);
  _DAT_d0000028 = 0x10000000;
  FUN_10001568(0x1c,1,0);
  pbVar9 = DAT_10000df0;
  pbVar14 = DAT_10000df0 + 10;
  pbVar17 = DAT_10000df0;
  do {
    bVar1 = *pbVar17;
    pbVar17 = pbVar17 + 1;
    FUN_100016b8((uint)bVar1);
    pbVar10 = DAT_10000df4;
    _DAT_d0000018 = 1 << (uint)bVar1;
    _DAT_d0000024 = _DAT_d0000018;
  } while (pbVar14 != pbVar17);
  pbVar14 = DAT_10000df4 + 4;
  pbVar17 = DAT_10000df4;
  do {
    bVar1 = *pbVar17;
    pbVar17 = pbVar17 + 1;
    _DAT_d0000018 = _DAT_d0000024;
    FUN_100016b8((uint)bVar1);
    _DAT_d0000018 = 1 << (uint)bVar1;
    _DAT_d0000024 = _DAT_d0000018;
  } while (pbVar17 != pbVar14);
  FUN_10001204();
  pcVar11 = DAT_10000df8;
  *DAT_10000df8 = '\0';
  pcVar12 = DAT_10000dfc;
  *DAT_10000dfc = '\0';
  pcVar8 = DAT_10000e00;
  pcVar8[0] = '\0';
  pcVar8[1] = '\0';
  pcVar8[2] = '\0';
  pcVar8[3] = '\0';
  FUN_100015e8(0x1d,0xc,1,DAT_10000e04);
  FUN_10001590(0x1b,0xc,1);
  FUN_10001590(0x1c,4,1);
  local_2c = 0;
  _DAT_d0000018 = 0x100;
  _DAT_d0000014 = 0x80;
  FUN_10001218(0,0,0x20);
  lVar19 = FUN_100025c8();
  iVar13 = DAT_10000e10;
  lVar19 = lVar19 + (ulonglong)DAT_10000e08;
  if (lVar19 < 0) {
    lVar19 = CONCAT44(DAT_10000e0c,0xffffffff);
  }
  local_3c = 0;
  lVar23 = 0;
  lVar21 = 0;
  bVar5 = false;
  local_54 = 0;
  bVar6 = false;
  cVar2 = *pcVar11;
  uVar18 = 0;
  iVar15 = local_54;
  bVar7 = false;
  do {
    local_50 = (uint)lVar21;
    local_4c = (int)((ulonglong)lVar21 >> 0x20);
    if (cVar2 == '\0') {
      if (bVar5) {
        uVar22 = FUN_100025c8();
        uVar16 = local_4c - (int)((ulonglong)uVar22 >> 0x20);
        uVar3 = (uint)(local_50 < (uint)uVar22);
        bVar5 = true;
        if (((int)(uVar16 - uVar3) < 1) && ((uVar16 != uVar3 || (local_50 == (uint)uVar22)))) {
          if (iVar15 < 1) {
            if (iVar15 != 0) {
              local_3c = -1;
              bVar5 = false;
              goto LAB_10000dbe;
            }
          }
          else {
            local_3c = 1;
          }
          goto LAB_10000dba;
        }
      }
      else {
LAB_10000dba:
        bVar5 = false;
      }
LAB_10000dbe:
      bVar4 = false;
      local_54 = iVar15;
      if (bVar7) {
LAB_10000c3e:
        local_44 = (int)((ulonglong)lVar23 >> 0x20);
        local_48 = (uint)lVar23;
        uVar22 = FUN_100025c8();
        uVar16 = local_44 - (int)((ulonglong)uVar22 >> 0x20);
        uVar3 = (uint)(local_48 < (uint)uVar22);
        if (((int)(uVar16 - uVar3) < 1) && ((uVar16 != uVar3 || (local_48 == (uint)uVar22)))) {
          bVar4 = false;
          if (local_3c != 0) {
            uVar16 = (uint)*(byte *)((int)&local_2c + uVar18);
            if (local_3c == -1) goto LAB_100010ac;
            goto LAB_10001052;
          }
        }
        else {
          bVar4 = true;
        }
      }
    }
    else {
      local_54 = (int)cVar2;
      *pcVar11 = '\0';
      if (!bVar7) {
        lVar23 = FUN_100025c8();
        lVar23 = lVar23 + (ulonglong)DAT_100010fc;
        if (lVar23 < 0) {
          lVar23 = CONCAT44(DAT_100010f4,0xffffffff);
        }
        local_3c = 0;
      }
      local_44 = (int)((ulonglong)lVar23 >> 0x20);
      local_48 = (uint)lVar23;
      if (bVar5) {
        local_54 = iVar15 + local_54;
        uVar22 = FUN_100025c8();
        uVar16 = local_4c - (int)((ulonglong)uVar22 >> 0x20);
        uVar3 = (uint)(local_50 < (uint)uVar22);
        if ((0 < (int)(uVar16 - uVar3)) || ((uVar16 == uVar3 && (local_50 != (uint)uVar22))))
        goto LAB_10000c3e;
        bVar4 = true;
        if (local_54 < 1) {
          bVar5 = false;
          if (local_54 == 0) goto LAB_10000c3e;
          uVar22 = FUN_100025c8();
          uVar16 = local_44 - (int)((ulonglong)uVar22 >> 0x20);
          uVar3 = (uint)(local_48 < (uint)uVar22);
          if (((int)(uVar16 - uVar3) < 1) && ((uVar16 != uVar3 || (local_48 == (uint)uVar22)))) {
            uVar16 = (uint)*(byte *)((int)&local_2c + uVar18);
            bVar5 = false;
            goto LAB_100010ac;
          }
          local_3c = -1;
          bVar5 = false;
          goto LAB_10000c62;
        }
        uVar22 = FUN_100025c8();
        uVar16 = local_44 - (int)((ulonglong)uVar22 >> 0x20);
        uVar3 = (uint)(local_48 < (uint)uVar22);
        if ((0 < (int)(uVar16 - uVar3)) || ((uVar16 == uVar3 && (local_48 != (uint)uVar22)))) {
          local_3c = 1;
          bVar5 = false;
          goto LAB_10000c62;
        }
        uVar16 = (uint)*(byte *)((int)&local_2c + uVar18);
        bVar5 = false;
LAB_10001052:
        iVar15 = uVar16 - 1;
        if (uVar16 != 0) {
          local_3c = 1;
          goto joined_r0x100010b6;
        }
        local_3c = 1;
        iVar15 = 9;
      }
      else {
        lVar21 = FUN_100025c8();
        lVar21 = lVar21 + (ulonglong)DAT_100010f8;
        if (lVar21 < 0) {
          lVar21 = CONCAT44(DAT_100010f4,0xffffffff);
        }
        local_4c = (int)((ulonglong)lVar21 >> 0x20);
        local_50 = (uint)lVar21;
        uVar22 = FUN_100025c8();
        uVar16 = local_4c - (int)((ulonglong)uVar22 >> 0x20);
        uVar3 = (uint)(local_50 < (uint)uVar22);
        if ((0 < (int)(uVar16 - uVar3)) || ((uVar16 == uVar3 && (local_50 != (uint)uVar22)))) {
          bVar5 = true;
          goto LAB_10000c3e;
        }
        if (0 < local_54) {
          uVar22 = FUN_100025c8();
          uVar16 = local_44 - (int)((ulonglong)uVar22 >> 0x20);
          uVar3 = (uint)(local_48 < (uint)uVar22);
          if (((int)(uVar16 - uVar3) < 1) && ((uVar16 != uVar3 || (local_48 == (uint)uVar22)))) {
            uVar16 = (uint)*(byte *)((int)&local_2c + uVar18);
            goto LAB_10001052;
          }
          local_3c = 1;
          bVar4 = true;
          goto LAB_10000c62;
        }
        uVar22 = FUN_100025c8();
        uVar16 = local_44 - (int)((ulonglong)uVar22 >> 0x20);
        uVar3 = (uint)(local_48 < (uint)uVar22);
        if ((0 < (int)(uVar16 - uVar3)) || ((uVar16 == uVar3 && (local_48 != (uint)uVar22)))) {
          local_3c = -1;
          bVar4 = true;
          goto LAB_10000c62;
        }
        uVar16 = (uint)*(byte *)((int)&local_2c + uVar18);
LAB_100010ac:
        iVar15 = uVar16 + 1;
        local_3c = -1;
joined_r0x100010b6:
        for (; 9 < iVar15; iVar15 = iVar15 + -10) {
        }
      }
      *(char *)((int)&local_2c + uVar18) = (char)iVar15;
      _DAT_d0000018 = 0x100;
      _DAT_d0000014 = 1 << pbVar9[iVar15];
      bVar4 = false;
    }
LAB_10000c62:
    if (*pcVar12 == '\0') {
LAB_10000d92:
      lVar20 = FUN_100025c8();
      lVar20 = lVar19 - lVar20;
    }
    else {
      *pcVar12 = '\0';
      if (2 < uVar18) {
        iVar15 = 1;
        pcVar8[uVar18] = '\x01';
        while ((&cStack_2d)[iVar15] == *(char *)(iVar13 + iVar15)) {
          iVar15 = iVar15 + 1;
          FUN_1000239c(0x32);
          if (iVar15 == 5) {
            FUN_10001218(0,0x20,0);
            _DAT_d0000014 = 0x400;
            *DAT_10000e14 = 1;
            return;
          }
        }
        iVar15 = 3;
        _DAT_d0000014 = 0x400;
        do {
          FUN_10001218(0x20,0,0);
          FUN_1000239c(200);
          FUN_10001218(0,0,0);
          iVar15 = iVar15 + -1;
          FUN_1000239c(200);
        } while (iVar15 != 0);
        pcVar8[0] = '\0';
        pcVar8[1] = '\0';
        pcVar8[2] = '\0';
        pcVar8[3] = '\0';
        _DAT_d0000018 = 0x100;
        _DAT_d0000014 = 0x80;
        local_2c = iVar15;
        FUN_10001218(0,0,0x20);
        lVar19 = FUN_100025c8();
        lVar19 = lVar19 + (ulonglong)DAT_10000e08;
        if (lVar19 < 0) {
          lVar19 = CONCAT44(DAT_10000e0c,0xffffffff);
        }
        uVar18 = 0;
        bVar6 = false;
        goto LAB_10000d92;
      }
      pcVar8[uVar18] = '\x01';
      uVar18 = uVar18 + 1 & 0xff;
      _DAT_d0000018 = 0x100;
      if (9 < *(byte *)((int)&local_2c + uVar18)) goto LAB_10000d92;
      _DAT_d0000014 = 1 << (uint)pbVar9[*(byte *)((int)&local_2c + uVar18)];
      lVar20 = FUN_100025c8();
      lVar20 = lVar19 - lVar20;
    }
    iVar15 = _DAT_d0000014;
    if (lVar20 < 1) {
      lVar19 = FUN_100025c8();
      lVar19 = lVar19 + (ulonglong)DAT_100010f0;
      if (lVar19 < 0) {
        lVar19 = CONCAT44(DAT_100010f4,0xffffffff);
      }
      bVar6 = (bool)(bVar6 ^ 1);
      _DAT_d0000018 = 0x400;
      if (*pcVar8 != '\0') {
        _DAT_d0000014 = 0x2000;
      }
      if (pcVar8[1] != '\0') {
        _DAT_d0000014 = 0x1000;
      }
      if (pcVar8[2] != '\0') {
        _DAT_d0000014 = 0x800;
      }
      if (pcVar8[3] != '\0') {
        _DAT_d0000014 = 0x400;
      }
      iVar15 = _DAT_d0000014;
      if ((pcVar8[uVar18] == '\0') && (iVar15 = 1 << pbVar10[uVar18], !bVar6)) {
        iVar15 = _DAT_d0000014;
        _DAT_d0000018 = 1 << pbVar10[uVar18];
      }
    }
    _DAT_d0000014 = iVar15;
    FUN_1000239c(2);
    cVar2 = *pcVar11;
    iVar15 = local_54;
    bVar7 = bVar4;
  } while( true );
}



/* ---------------------------------------------------------------- */
/* FUN_10001100 @ 10001100  (220 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10001100(void)

{
  int *piVar1;
  uint *puVar2;
  undefined4 *puVar3;
  byte bVar4;
  ushort uVar5;
  int iVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  uint uVar9;
  uint *puVar10;
  uint uVar11;
  uint local_28;
  uint local_24;
  undefined4 local_20;
  undefined4 local_1c;
  
  puVar3 = DAT_100011e4;
  puVar2 = DAT_100011e0;
  piVar1 = DAT_100011dc;
  iVar6 = FUN_100032bc(DAT_100011e8,DAT_100011e4,DAT_100011e0,DAT_100011dc,0x10,1,1);
  if (iVar6 == 0) {
    FUN_10002928();
  }
  puVar10 = (uint *)*puVar3;
  iVar6 = *piVar1;
  uVar11 = *puVar2;
  FUN_10001534(0x10,((uint)((int)puVar10 + DAT_100011ec) >> 0x14) + 6 & 0xff);
  FUN_1000316c(puVar10,uVar11,0x10,1,1);
  local_28 = 0x10000;
  local_24 = ((iVar6 + 3) * 0x1000 | iVar6 << 7) & DAT_100011f0;
  local_1c = DAT_100011f4;
  local_20 = DAT_100011f8;
  FUN_100027d8(5);
  uVar7 = FUN_10002d14();
  uVar7 = FUN_10002cde(uVar7,DAT_100011fc);
  uVar7 = FUN_10002cd8(uVar7,0x3b000000);
  uVar5 = FUN_10002d64();
  uVar9 = 0;
  if (uVar5 != 0) {
    uVar8 = FUN_10002d14(uVar5);
    uVar7 = FUN_10002cd2(uVar7,uVar8);
    FUN_10002d06(uVar7,0x43800000);
    bVar4 = FUN_10002d64();
    uVar9 = (uint)bVar4 << 8;
  }
  local_28 = (uint)uVar5 << 0x10 | uVar9;
  FUN_10003220(puVar10,uVar11,iVar6,&local_28);
  *puVar10 = 1 << (uVar11 & 0xff) | *puVar10;
  *DAT_10001200 = 1;
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10001204 @ 10001204  (16 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10001204(void)

{
  if (*DAT_10001214 == '\0') {
    FUN_10001100();
  }
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10001218 @ 10001218  (72 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10001218(int param_1,int param_2,uint param_3)

{
  if (*DAT_10001260 == '\0') {
    FUN_10001100();
  }
  do {
  } while ((1 << (*DAT_10001268 + 0x10U & 0xff) & *(uint *)(*DAT_10001264 + 4)) != 0);
  *(uint *)((*DAT_10001268 + 4) * 4 + *DAT_10001264) =
       (param_2 << 0x10 | param_3 | param_1 << 8) << 8;
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10001534 @ 10001534  (38 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10001534(int param_1,undefined4 param_2)

{
  *(uint *)(param_1 * 4 + DAT_10001560) = (*(uint *)(param_1 * 4 + DAT_1000155c) ^ 0x40) & 0xc0;
  *(undefined4 *)(param_1 * 8 + DAT_10001564 + 4) = param_2;
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10001568 @ 10001568  (30 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10001568(int param_1,int param_2,int param_3)

{
  *(uint *)(param_1 * 4 + DAT_1000158c) =
       ((param_3 << 2 | param_2 << 3) ^ *(uint *)(param_1 * 4 + DAT_10001588)) & 0xc;
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10001590 @ 10001590  (78 bytes) */
/* ---------------------------------------------------------------- */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_10001590(uint param_1,int param_2,int param_3)

{
  int iVar1;
  
  param_2 = param_2 << ((param_1 & 7) << 2);
  iVar1 = (-(uint)(_DAT_d0000000 == 0) & 0xffffffd0) + DAT_100015e0;
  *(int *)(((param_1 >> 3) + 0x3c) * 4 + DAT_100015e4) = param_2;
  iVar1 = iVar1 + (param_1 >> 3) * 4;
  if (param_3 == 0) {
    *(int *)(iVar1 + 0x3000) = param_2;
  }
  else {
    *(int *)(iVar1 + 0x2000) = param_2;
  }
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_100015e8 @ 100015e8  (190 bytes) */
/* ---------------------------------------------------------------- */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_100015e8(uint param_1,int param_2,int param_3,int param_4)

{
  int iVar1;
  int iVar2;
  
  iVar2 = DAT_100016a8;
  iVar1 = _DAT_d0000000 * 4;
  if (*(int *)(DAT_100016a8 + iVar1) == 0) {
    if (param_4 != 0) {
      *(int *)(DAT_100016a8 + iVar1) = param_4;
      FUN_10001958(0xd,DAT_100016b4,0);
    }
  }
  else {
    if (param_4 == 0) {
      FUN_10001b4c(0xd,DAT_100016b4);
    }
    *(int *)(iVar2 + iVar1) = param_4;
  }
  param_2 = param_2 << ((param_1 & 7) << 2);
  iVar2 = (-(uint)(_DAT_d0000000 == 0) & 0xffffffd0) + DAT_100016ac;
  *(int *)(((param_1 >> 3) + 0x3c) * 4 + DAT_100016b0) = param_2;
  iVar2 = iVar2 + (param_1 >> 3) * 4;
  if (param_3 == 0) {
    *(int *)(iVar2 + 0x3000) = param_2;
  }
  else {
    *(int *)(iVar2 + 0x2000) = param_2;
    FUN_100018e0(0xd,1);
  }
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_100016b8 @ 100016b8  (50 bytes) */
/* ---------------------------------------------------------------- */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_100016b8(uint param_1)

{
  _DAT_d0000018 = 1 << (param_1 & 0xff);
  _DAT_d0000028 = _DAT_d0000018;
  *(uint *)(param_1 * 4 + DAT_100016f0) = (*(uint *)(param_1 * 4 + DAT_100016ec) ^ 0x40) & 0xc0;
  *(undefined4 *)(param_1 * 8 + DAT_100016f4 + 4) = 5;
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10001704 @ 10001704  (48 bytes) */
/* ---------------------------------------------------------------- */

undefined4 FUN_10001704(int param_1)

{
  bool bVar1;
  undefined4 uVar2;
  int extraout_r2;
  int unaff_r4;
  int unaff_r6;
  
  FUN_100030f4(s_____PANIC_____10005390);
  if (param_1 != 0) {
    FUN_10002e38(param_1);
    FUN_100030f4(&DAT_100053a0);
  }
  uVar2 = FUN_10002e68(1);
  *(short *)(extraout_r2 + unaff_r6) = (short)uVar2;
  *(short *)(unaff_r4 + unaff_r6) = (short)((int)uVar2 >> 0x1f);
  uVar2 = 0;
  bVar1 = (bool)isCurrentModePrivileged();
  if (bVar1) {
    uVar2 = isIRQinterruptsEnabled();
  }
  disableIRQinterrupts();
  do {
  } while (*DAT_10001748 == 0);
  DataMemoryBarrier(0x1f);
  return uVar2;
}



/* ---------------------------------------------------------------- */
/* FUN_10001734 @ 10001734  (20 bytes) */
/* ---------------------------------------------------------------- */

undefined4 FUN_10001734(void)

{
  bool bVar1;
  undefined4 uVar2;
  
  uVar2 = 0;
  bVar1 = (bool)isCurrentModePrivileged();
  if (bVar1) {
    uVar2 = isIRQinterruptsEnabled();
  }
  disableIRQinterrupts();
  do {
  } while (*DAT_10001748 == 0);
  DataMemoryBarrier(0x1f);
  return uVar2;
}



/* ---------------------------------------------------------------- */
/* FUN_1000174c @ 1000174c  (16 bytes) */
/* ---------------------------------------------------------------- */

void FUN_1000174c(uint param_1)

{
  bool bVar1;
  
  DataMemoryBarrier(0x1f);
  *DAT_1000175c = 0;
  bVar1 = (bool)isCurrentModePrivileged();
  if (bVar1) {
    enableIRQinterrupts((param_1 & 1) == 1);
  }
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10001760 @ 10001760  (64 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10001760(int param_1,uint param_2,undefined4 param_3)

{
  byte bVar1;
  bool bVar2;
  uint *puVar3;
  uint uVar4;
  uint uVar5;
  
  puVar3 = DAT_100017a0;
  uVar5 = 0;
  bVar2 = (bool)isCurrentModePrivileged();
  if (bVar2) {
    uVar5 = isIRQinterruptsEnabled();
  }
  disableIRQinterrupts();
  do {
  } while (*DAT_100017a0 == 0);
  DataMemoryBarrier(0x1f);
  bVar1 = *(byte *)(param_1 + (param_2 >> 3));
  uVar4 = 1 << (param_2 & 7);
  if ((uVar4 & bVar1) == 0) {
    *(byte *)(param_1 + (param_2 >> 3)) = bVar1 | (byte)uVar4;
    DataMemoryBarrier(0x1f);
    *puVar3 = uVar4 & bVar1;
    bVar2 = (bool)isCurrentModePrivileged();
    if (bVar2) {
      enableIRQinterrupts((uVar5 & 1) == 1);
    }
    return;
  }
                    /* WARNING: Subroutine does not return */
  FUN_10001704(param_3);
}



/* ---------------------------------------------------------------- */
/* FUN_100017a4 @ 100017a4  (130 bytes) */
/* ---------------------------------------------------------------- */

uint FUN_100017a4(int param_1,int param_2,uint param_3,uint param_4,undefined4 param_5)

{
  byte bVar1;
  bool bVar2;
  uint uVar3;
  uint uVar4;
  
  uVar4 = 0;
  bVar2 = (bool)isCurrentModePrivileged();
  if (bVar2) {
    uVar4 = isIRQinterruptsEnabled();
  }
  disableIRQinterrupts();
  do {
  } while (*DAT_10001828 == 0);
  DataMemoryBarrier(0x1f);
  if (param_4 < param_3) {
    DataMemoryBarrier(0x1f);
    *DAT_10001828 = 0;
    bVar2 = (bool)isCurrentModePrivileged();
    if (bVar2) {
      enableIRQinterrupts((uVar4 & 1) == 1);
    }
    param_3 = 0xffffffff;
  }
  else {
    do {
      uVar3 = 1 << (param_3 & 7);
      bVar1 = *(byte *)(param_1 + (param_3 >> 3));
      if ((uVar3 & bVar1) == 0) {
        *(byte *)(param_1 + (param_3 >> 3)) = bVar1 | (byte)uVar3;
        goto LAB_100017ea;
      }
      param_3 = param_3 + 1;
    } while (param_3 <= param_4);
    param_3 = 0xffffffff;
LAB_100017ea:
    DataMemoryBarrier(0x1f);
    *DAT_10001828 = 0;
    bVar2 = (bool)isCurrentModePrivileged();
    if (bVar2) {
      enableIRQinterrupts((uVar4 & 1) == 1);
    }
    if (-1 < (int)param_3) {
      return param_3;
    }
  }
  if (param_2 == 0) {
    return param_3;
  }
                    /* WARNING: Subroutine does not return */
  FUN_10001704(param_5);
}



/* ---------------------------------------------------------------- */
/* FUN_1000182c @ 1000182c  (50 bytes) */
/* ---------------------------------------------------------------- */

void FUN_1000182c(int param_1,uint param_2)

{
  bool bVar1;
  int *piVar2;
  uint uVar3;
  
  piVar2 = DAT_10001860;
  uVar3 = 0;
  bVar1 = (bool)isCurrentModePrivileged();
  if (bVar1) {
    uVar3 = isIRQinterruptsEnabled();
  }
  disableIRQinterrupts();
  do {
  } while (*DAT_10001860 == 0);
  DataMemoryBarrier(0x1f);
  *(byte *)(param_1 + (param_2 >> 3)) =
       *(byte *)(param_1 + (param_2 >> 3)) & ~(byte)(1 << (param_2 & 7));
  DataMemoryBarrier(0x1f);
  *piVar2 = 0;
  bVar1 = (bool)isCurrentModePrivileged();
  if (bVar1) {
    enableIRQinterrupts((uVar3 & 1) == 1);
  }
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10001864 @ 10001864  (18 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10001864(void)

{
  byte bVar1;
  
  bVar1 = *DAT_10001878 + 1;
  if (0x17 < bVar1) {
    bVar1 = 0x10;
  }
  *DAT_10001878 = bVar1;
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_1000187c @ 1000187c  (26 bytes) */
/* ---------------------------------------------------------------- */

void FUN_1000187c(undefined4 param_1)

{
  FUN_100017a4(DAT_1000189c,param_1,0x18,0x1f,DAT_10001898);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_100018a0 @ 100018a0  (18 bytes) */
/* ---------------------------------------------------------------- */

void FUN_100018a0(void)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  
  puVar1 = DAT_100018b8;
  puVar2 = DAT_100018b4;
  do {
    DataMemoryBarrier(0x1f);
    *puVar2 = 0;
    puVar2 = puVar2 + 1;
  } while (puVar2 != puVar1);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_100018e0 @ 100018e0  (32 bytes) */
/* ---------------------------------------------------------------- */

void FUN_100018e0(uint param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  
  piVar1 = DAT_10001900;
  iVar2 = 1 << (param_1 & 0x1f);
  if (param_2 == 0) {
    DAT_10001900[0x20] = iVar2;
  }
  else {
    DAT_10001900[0x60] = iVar2;
    *piVar1 = iVar2;
  }
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10001904 @ 10001904  (72 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10001904(int param_1,int param_2)

{
  bool bVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  
  uVar4 = 0;
  bVar1 = (bool)isCurrentModePrivileged();
  if (bVar1) {
    uVar4 = isIRQinterruptsEnabled();
  }
  disableIRQinterrupts();
  do {
  } while (*DAT_1000194c == 0);
  DataMemoryBarrier(0x1f);
  iVar3 = (param_1 + 0x10) * 4;
  iVar2 = *(int *)(*(int *)(DAT_10001950 + 8) + iVar3);
  if ((iVar2 != DAT_10001954) && (param_2 != iVar2)) {
    FUN_10002928();
  }
  *(int *)(*(int *)(DAT_10001950 + 8) + iVar3) = param_2;
  DataMemoryBarrier(0x1f);
  DataMemoryBarrier(0x1f);
  *DAT_1000194c = 0;
  bVar1 = (bool)isCurrentModePrivileged();
  if (bVar1) {
    enableIRQinterrupts((uVar4 & 1) == 1);
  }
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10001958 @ 10001958  (452 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10001958(int param_1,undefined4 param_2,uint param_3)

{
  char cVar1;
  bool bVar2;
  undefined1 uVar3;
  ushort uVar4;
  char *pcVar5;
  undefined4 uVar6;
  uint uVar7;
  undefined4 *puVar8;
  undefined4 *puVar9;
  undefined4 *puVar10;
  int iVar11;
  int iVar12;
  undefined4 *puVar13;
  uint uVar14;
  ushort local_2c;
  
  pcVar5 = DAT_10001b20;
  uVar14 = 0;
  bVar2 = (bool)isCurrentModePrivileged();
  if (bVar2) {
    uVar14 = isIRQinterruptsEnabled();
  }
  disableIRQinterrupts();
  do {
  } while (*DAT_10001b1c == 0);
  DataMemoryBarrier(0x1f);
  iVar11 = (int)*DAT_10001b20;
  if (iVar11 < 0) {
    FUN_10002928();
    iVar11 = (int)*pcVar5;
  }
  puVar10 = DAT_10001b24;
  puVar13 = DAT_10001b24 + iVar11 * 3;
  *pcVar5 = *(char *)((int)puVar13 + 6);
  uVar6 = DAT_10001b38;
  iVar12 = (param_1 + 0x10) * 4;
  uVar7 = *(uint *)(*(int *)(DAT_10001b28 + 8) + iVar12);
  uVar3 = (undefined1)param_3;
  if (uVar7 - (int)puVar10 < 0x30) {
    puVar8 = (undefined4 *)(uVar7 & 0xfffffffe);
    if (param_3 < *(byte *)((int)puVar8 + 7)) {
      do {
        puVar9 = puVar8;
        cVar1 = *(char *)((int)puVar9 + 6);
        if (cVar1 < 0) {
          local_2c = (ushort)DAT_10001b48;
          goto LAB_10001aa2;
        }
        puVar8 = puVar10 + cVar1 * 3;
      } while (param_3 < *(byte *)((int)(puVar10 + cVar1 * 3) + 7));
      local_2c = (ushort)(((int)puVar9 +
                           ((((int)((uint)*(ushort *)(puVar9 + 1) << 0x15) >> 0x14) + 4) -
                           (int)(puVar10 + iVar11 * 3 + 1)) & 0xfffU) >> 1) | 0xe000;
LAB_10001aa2:
      *(char *)((int)puVar9 + 6) = (char)iVar11;
      *(ushort *)(puVar9 + 1) =
           (ushort)(((int)puVar13 + (-4 - (int)(puVar9 + 1)) & 0xfffU) >> 1) | (ushort)uVar6;
      puVar10 = puVar10 + iVar11 * 3;
      *(ushort *)(puVar10 + 1) = local_2c;
      uVar6 = DAT_10001b44;
      *(char *)((int)puVar10 + 6) = cVar1;
      *puVar10 = uVar6;
      *(undefined1 *)((int)puVar10 + 7) = uVar3;
      puVar10[2] = param_2;
    }
    else {
      uVar7 = (((int)puVar8 - (int)puVar10) * 2 + (uint)(puVar10 <= puVar8)) * DAT_10001b40;
      *(short *)(puVar10 + iVar11 * 3) = (short)DAT_10001b30;
      uVar4 = (ushort)DAT_10001b38;
      *(ushort *)((int)puVar13 + 2) =
           (ushort)(((DAT_10001b34 + -4) - ((int)puVar10 + iVar11 * 0xc + 2) & 0xfffU) >> 1) | uVar4
      ;
      *(char *)((int)puVar13 + 6) = (char)(uVar7 >> 0x14);
      *(ushort *)(puVar13 + 1) =
           uVar4 | (ushort)(((int)puVar8 + (-4 - (int)(puVar10 + iVar11 * 3 + 1)) & 0xfffU) >> 1);
      puVar13[2] = param_2;
      *(undefined1 *)((int)puVar13 + 7) = uVar3;
      uVar7 = (uint)puVar13 | 1;
      *puVar8 = DAT_10001b44;
    }
  }
  else {
    if (uVar7 != DAT_10001b2c) {
      FUN_10002928();
    }
    *(short *)(puVar10 + iVar11 * 3) = (short)DAT_10001b30;
    *(ushort *)((int)puVar10 + iVar11 * 0xc + 2) =
         (ushort)(((DAT_10001b34 + -4) - ((int)puVar10 + iVar11 * 0xc + 2) & 0xfffU) >> 1) |
         (ushort)DAT_10001b38;
    *(short *)(puVar10 + iVar11 * 3 + 1) = (short)DAT_10001b3c;
    *(undefined1 *)((int)puVar10 + iVar11 * 0xc + 6) = 0xff;
    *(undefined1 *)((int)puVar10 + iVar11 * 0xc + 7) = uVar3;
    puVar10[iVar11 * 3 + 2] = param_2;
    uVar7 = (uint)puVar13 | 1;
  }
  *(uint *)(*(int *)(DAT_10001b28 + 8) + iVar12) = uVar7;
  DataMemoryBarrier(0x1f);
  DataMemoryBarrier(0x1f);
  *DAT_10001b1c = 0;
  bVar2 = (bool)isCurrentModePrivileged();
  if (bVar2) {
    enableIRQinterrupts((uVar14 & 1) == 1);
  }
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10001b4c @ 10001b4c  (418 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10001b4c(uint param_1,uint param_2)

{
  char cVar1;
  bool bVar2;
  uint uVar3;
  uint *puVar4;
  undefined1 *puVar5;
  uint uVar6;
  int iVar7;
  ushort uVar8;
  uint uVar9;
  uint uVar10;
  uint uVar11;
  uint uVar12;
  uint uVar13;
  uint uVar14;
  int iVar15;
  uint uVar16;
  int iVar17;
  uint uVar18;
  
  uVar3 = DAT_10001cfc;
  uVar11 = DAT_10001cf8;
  uVar18 = 0;
  bVar2 = (bool)isCurrentModePrivileged();
  if (bVar2) {
    uVar18 = isIRQinterruptsEnabled();
  }
  disableIRQinterrupts();
  do {
  } while (*DAT_10001cf0 == 0);
  DataMemoryBarrier(0x1f);
  iVar17 = (param_1 + 0x10) * 4;
  uVar16 = *(uint *)(*(int *)(DAT_10001cf4 + 8) + iVar17);
  uVar14 = uVar16;
  if (((uVar16 == DAT_10001cf8) || (uVar14 = DAT_10001cf8, uVar16 == param_2)) ||
     (uVar14 = uVar16, 0x2f < uVar16 - DAT_10001cfc)) goto LAB_10001c0a;
  uVar12 = *DAT_10001d00;
  uVar13 = 1 << (param_1 & 0x1f);
  DAT_10001d00[0x20] = uVar13;
  DataMemoryBarrier(0x1f);
  uVar14 = 0;
  bVar2 = (bool)isCurrentModePrivileged();
  if (bVar2) {
    uVar14 = getCurrentExceptionNumber();
    uVar14 = uVar14 & 0x1ff;
  }
  if (uVar14 == 0) {
    uVar9 = uVar16 & 0xfffffffe;
    uVar6 = *(uint *)(uVar9 + 8);
    if (param_2 != uVar6) goto LAB_10001bec;
    iVar7 = (int)*(char *)(uVar9 + 6);
    if (-1 < iVar7) goto LAB_10001c60;
LAB_10001cb2:
    puVar5 = DAT_10001d08;
    *(undefined1 *)(uVar9 + 6) = *DAT_10001d08;
    *puVar5 = (char)(((uVar9 - uVar3) * 2 + (uint)(uVar3 <= uVar9)) * DAT_10001d10 >> 0x14);
  }
  else {
    if (param_1 + 0x10 != uVar14) {
      FUN_10002928();
    }
    uVar9 = uVar16 & 0xfffffffe;
    uVar6 = *(uint *)(uVar9 + 8);
    if (param_2 == uVar6) {
      iVar7 = (int)*(char *)(uVar9 + 6);
      if (-1 < iVar7) {
LAB_10001c60:
        iVar15 = uVar3 + iVar7 * 0xc;
        *(undefined4 *)(uVar9 + 8) = *(undefined4 *)(iVar15 + 8);
        *(undefined1 *)(uVar9 + 7) = *(undefined1 *)(iVar15 + 7);
        cVar1 = *(char *)(iVar15 + 6);
        *(char *)(uVar9 + 6) = cVar1;
        if (cVar1 < '\0') {
          uVar8 = (ushort)DAT_10001d0c;
        }
        else {
          uVar8 = (ushort)(((uVar3 - uVar9) +
                            ((int)((uint)*(ushort *)(iVar15 + 4) << 0x15) >> 0x14) + iVar7 * 0xc &
                           0xfff) >> 1) | (ushort)DAT_10001d04;
        }
        *(ushort *)(uVar9 + 4) = uVar8;
        puVar5 = DAT_10001d08;
        *(undefined1 *)(uVar3 + iVar7 * 0xc + 6) = *DAT_10001d08;
        *puVar5 = (char)iVar7;
        uVar11 = uVar16;
        goto LAB_10001bf8;
      }
    }
    else {
LAB_10001bec:
      do {
        uVar10 = uVar9;
        if (*(char *)(uVar10 + 6) < 0) {
          uVar9 = uVar10;
          uVar11 = uVar16;
          if (param_2 != uVar6) goto LAB_10001bf8;
          goto LAB_10001ca4;
        }
        uVar9 = uVar3 + *(char *)(uVar10 + 6) * 0xc;
        uVar6 = *(uint *)(uVar9 + 8);
      } while (param_2 != uVar6);
      iVar7 = (int)*(char *)(uVar9 + 6);
      if (-1 < iVar7) goto LAB_10001c60;
LAB_10001ca4:
      if (uVar14 == 0) {
        *(undefined1 *)(uVar10 + 6) = 0xff;
        *(short *)(uVar10 + 4) = (short)DAT_10001d0c;
        uVar11 = uVar16;
        goto LAB_10001cb2;
      }
    }
    uVar11 = (DAT_10001d14 - uVar9) - 8;
    *(ushort *)(uVar9 + 4) = (ushort)DAT_10001d18 | (ushort)((uVar11 & 0x7fffff) >> 0xc);
    *(ushort *)(uVar9 + 6) = (ushort)(uVar11 >> 1) | (ushort)DAT_10001d1c;
    uVar11 = uVar16;
  }
LAB_10001bf8:
  puVar4 = DAT_10001d00;
  uVar14 = uVar11;
  if ((uVar12 & 1 << (param_1 & 0xff)) == 0) {
    DAT_10001d00[0x20] = uVar13;
  }
  else {
    DAT_10001d00[0x60] = uVar13;
    *puVar4 = uVar13;
  }
LAB_10001c0a:
  *(uint *)(*(int *)(DAT_10001cf4 + 8) + iVar17) = uVar14;
  DataMemoryBarrier(0x1f);
  DataMemoryBarrier(0x1f);
  *DAT_10001cf0 = 0;
  bVar2 = (bool)isCurrentModePrivileged();
  if (bVar2) {
    enableIRQinterrupts((uVar18 & 1) == 1);
  }
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10001dd4 @ 10001dd4  (12 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10001dd4(int *param_1,int param_2)

{
  *param_1 = (param_2 + DAT_10001de0) * 4;
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10001de4 @ 10001de4  (70 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10001de4(void)

{
  int *piVar1;
  undefined4 uVar2;
  int *piVar3;
  
  piVar1 = DAT_10001e30;
  piVar3 = DAT_10001e2c;
  do {
    if (piVar1 <= piVar3) {
      return;
    }
    while (*piVar3 == 0) {
      uVar2 = FUN_10001864();
      FUN_10001dd4(piVar3,uVar2);
      *(undefined1 *)(piVar3 + 1) = 0xff;
      DataMemoryBarrier(0x1f);
      piVar3 = piVar3 + 2;
      if (piVar1 <= piVar3) {
        return;
      }
    }
    uVar2 = FUN_10001864();
    FUN_10001dd4(piVar3,uVar2);
    *(undefined2 *)(piVar3 + 1) = 0xff;
    DataMemoryBarrier(0x1f);
    piVar3 = piVar3 + 2;
  } while( true );
}



/* ---------------------------------------------------------------- */
/* FUN_10001e34 @ 10001e34  (26 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10001e34(int param_1)

{
  undefined4 uVar1;
  
  uVar1 = FUN_10001864();
  FUN_10001dd4(param_1,uVar1);
  *(undefined1 *)(param_1 + 4) = 0xff;
  DataMemoryBarrier(0x1f);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10001e50 @ 10001e50  (24 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10001e50(int *param_1)

{
  int iVar1;
  
  iVar1 = FUN_1000187c(1);
  *param_1 = (iVar1 + DAT_10001e68) * 4;
  DataMemoryBarrier(0x1f);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10001e9c @ 10001e9c  (750 bytes) */
/* ---------------------------------------------------------------- */

/* WARNING: Removing unreachable block (ram,0x10001f30) */
/* WARNING: Removing unreachable block (ram,0x10001f64) */
/* WARNING: Removing unreachable block (ram,0x10001fe4) */
/* WARNING: Removing unreachable block (ram,0x10002018) */

void FUN_10001e9c(void)

{
  bool bVar1;
  code *pcVar2;
  short sVar3;
  undefined4 uVar4;
  uint uVar5;
  int iVar6;
  longlong *plVar7;
  uint uVar8;
  undefined4 uVar9;
  int iVar10;
  short *psVar11;
  int iVar12;
  int iVar13;
  int iVar14;
  short *psVar15;
  short *psVar16;
  longlong lVar17;
  longlong lVar18;
  uint local_40;
  int local_3c;
  
  uVar8 = 0;
  bVar1 = (bool)isCurrentModePrivileged();
  if (bVar1) {
    uVar8 = getCurrentExceptionNumber();
    uVar8 = uVar8 & 0x1ff;
  }
  uVar8 = uVar8 & 3;
  iVar10 = *(int *)(DAT_1000218c + uVar8 * 4);
  *DAT_10002190 = 1 << uVar8;
  pcVar2 = DAT_10002194;
  do {
    *(int *)(DAT_10002198 + 0x34) = 1 << uVar8;
    sVar3 = *(short *)(iVar10 + 8);
    if (sVar3 < 0) {
LAB_10001ed8:
      if (*(short *)(iVar10 + 4) < 0) goto LAB_10001ee2;
LAB_1000203e:
      uVar5 = 0;
      bVar1 = (bool)isCurrentModePrivileged();
      if (bVar1) {
        uVar5 = isIRQinterruptsEnabled();
      }
      disableIRQinterrupts();
      do {
      } while (**(int **)(iVar10 + 0x10) == 0);
      DataMemoryBarrier(0x1f);
      sVar3 = *(short *)(iVar10 + 4);
      *(undefined2 *)(iVar10 + 4) = 0xffff;
      iVar14 = (int)sVar3;
      DataMemoryBarrier(0x1f);
      **(undefined4 **)(iVar10 + 0x10) = 0;
      bVar1 = (bool)isCurrentModePrivileged();
      if (bVar1) {
        enableIRQinterrupts((uVar5 & 1) == 1);
      }
      if (iVar14 < 0) goto LAB_10001ee2;
      iVar13 = *(int *)(iVar10 + 0x14);
      do {
        psVar15 = (short *)(iVar13 + iVar14 * 0x18);
        sVar3 = *(short *)(iVar10 + 8);
        iVar12 = (int)sVar3;
        psVar16 = (short *)(iVar10 + 8);
        if (-1 < iVar12) {
          while( true ) {
            psVar11 = (short *)(iVar13 + iVar12 * 0x18);
            if (*(int *)(psVar15 + 6) < *(int *)(psVar11 + 6)) break;
            if (((*(int *)(psVar11 + 6) == *(int *)(psVar15 + 6)) &&
                (*(uint *)(psVar15 + 4) < *(uint *)(psVar11 + 4))) ||
               (iVar12 = (int)*psVar11, psVar16 = psVar11, iVar12 < 0)) break;
          }
          sVar3 = (short)iVar12;
        }
        *psVar16 = (short)iVar14;
        iVar14 = (int)*psVar15;
        *psVar15 = sVar3;
      } while (-1 < iVar14);
      if (*(char *)(iVar10 + 6) == '\0') goto LAB_10001eea;
LAB_100020c8:
      *(undefined1 *)(iVar10 + 6) = 0;
      iVar14 = (int)*(short *)(iVar10 + 8);
      if (iVar14 == -1) {
        return;
      }
      iVar12 = *(int *)(iVar10 + 0x14);
      iVar13 = iVar14;
      psVar16 = (short *)(iVar10 + 8);
      do {
        while( true ) {
          psVar15 = (short *)(iVar12 + iVar13 * 0x18);
          sVar3 = *(short *)(iVar12 + iVar13 * 0x18);
          iVar6 = (int)sVar3;
          if (-1 < psVar15[1]) break;
          psVar15[4] = -1;
          psVar15[5] = -1;
          psVar15[6] = -1;
          psVar15[7] = -1;
          if (iVar13 != iVar14) {
            *psVar16 = sVar3;
            *psVar15 = *(short *)(iVar10 + 8);
            *(short *)(iVar10 + 8) = (short)iVar13;
          }
          iVar14 = iVar13;
          iVar13 = iVar6;
          if (iVar6 == -1) goto LAB_10001eee;
        }
        iVar13 = iVar6;
        psVar16 = psVar15;
      } while (iVar6 != -1);
    }
    else {
      psVar16 = (short *)(*(int *)(iVar10 + 0x14) + sVar3 * 0x18);
      uVar5 = *(uint *)(psVar16 + 4);
      iVar14 = *(int *)(psVar16 + 6);
      lVar18 = *(longlong *)(psVar16 + 4);
      lVar17 = FUN_100025b8();
      if (lVar17 < lVar18) goto LAB_10001ed8;
      if (iVar14 < 0) {
LAB_1000213e:
        *(short *)(iVar10 + 8) = *psVar16;
        uVar5 = 0;
        bVar1 = (bool)isCurrentModePrivileged();
        if (bVar1) {
          uVar5 = isIRQinterruptsEnabled();
        }
        disableIRQinterrupts();
        do {
        } while (**(int **)(iVar10 + 0x10) == 0);
        DataMemoryBarrier(0x1f);
        *psVar16 = *(short *)(iVar10 + 2);
        *(short *)(iVar10 + 2) = sVar3;
        DataMemoryBarrier(0x1f);
        **(undefined4 **)(iVar10 + 0x10) = 0;
        bVar1 = (bool)isCurrentModePrivileged();
        if (bVar1) {
          enableIRQinterrupts((uVar5 & 1) == 1);
        }
        goto LAB_10001ed8;
      }
      plVar7 = *(longlong **)(psVar16 + 10);
      if (*(code **)(psVar16 + 8) == pcVar2) {
        iVar13 = (**(code **)(plVar7 + 2))(plVar7);
        if (iVar13 == 0) goto LAB_1000213e;
        lVar18 = *plVar7;
      }
      else {
        lVar18 = (**(code **)(psVar16 + 8))(CONCAT22(*(undefined2 *)(iVar10 + 8),psVar16[1]),plVar7)
        ;
      }
      local_3c = (int)((ulonglong)lVar18 >> 0x20);
      local_40 = (uint)lVar18;
      if (lVar18 == 0) goto LAB_1000213e;
      lVar17 = CONCAT44((iVar14 - local_3c) - (uint)(uVar5 < local_40),uVar5 - local_40);
      if (-1 < lVar18) {
        lVar17 = FUN_100025b8(DAT_10002198);
        lVar17 = lVar17 + lVar18;
      }
      *(longlong *)(psVar16 + 4) = lVar17;
      iVar14 = (int)*psVar16;
      if (iVar14 < 0) goto LAB_10001ed8;
      iVar13 = *(int *)(iVar10 + 0x14);
      iVar12 = iVar13 + iVar14 * 0x18;
      plVar7 = (longlong *)(iVar12 + 8);
      uVar9 = *(undefined4 *)plVar7;
      uVar4 = *(undefined4 *)(iVar12 + 0xc);
      if (lVar17 < *plVar7) goto LAB_10001ed8;
      *(short *)(iVar10 + 8) = *psVar16;
      psVar15 = (short *)(iVar10 + 8);
      while ((psVar11 = (short *)(iVar13 + iVar14 * 0x18), CONCAT44(uVar4,uVar9) <= lVar17 &&
             (iVar14 = (int)*psVar11, psVar15 = psVar11, -1 < iVar14))) {
        iVar12 = iVar13 + iVar14 * 0x18;
        uVar9 = *(undefined4 *)(iVar12 + 8);
        uVar4 = *(undefined4 *)(iVar12 + 0xc);
      }
      *psVar16 = (short)iVar14;
      *psVar15 = sVar3;
      if (-1 < *(short *)(iVar10 + 4)) goto LAB_1000203e;
LAB_10001ee2:
      if (*(char *)(iVar10 + 6) != '\0') goto LAB_100020c8;
LAB_10001eea:
      iVar14 = (int)*(short *)(iVar10 + 8);
    }
LAB_10001eee:
    if (iVar14 < 0) {
      return;
    }
    iVar14 = *(int *)(iVar10 + 0x14) + iVar14 * 0x18;
    plVar7 = (longlong *)(iVar14 + 8);
    iVar13 = *(int *)plVar7;
    lVar18 = *plVar7;
    if (((iVar13 != -1) || (*(int *)(iVar14 + 0xc) != -1)) &&
       ((iVar14 = DAT_10002198 + uVar8 * 4,
        (uint)(iVar13 - *(int *)(DAT_10002198 + 0x28)) <
        (uint)(*(int *)(iVar14 + 0x10) - *(int *)(DAT_10002198 + 0x28)) ||
        ((*(uint *)(DAT_10002198 + 0x20) & 1 << uVar8) == 0)))) {
      *(int *)(iVar14 + 0x10) = iVar13;
    }
    lVar17 = FUN_100025b8(DAT_10002198);
    if (lVar17 < lVar18) {
      return;
    }
  } while( true );
}



/* ---------------------------------------------------------------- */
/* FUN_1000219c @ 1000219c  (186 bytes) */
/* ---------------------------------------------------------------- */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_1000219c(void)

{
  undefined1 *puVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  int iVar5;
  undefined2 *puVar6;
  
  iVar2 = DAT_10002260;
  puVar1 = DAT_10002258;
  if (*(int *)(DAT_10002258 + 0x10) == 0) {
    FUN_100025a4(DAT_10002260,3);
    *(int *)(puVar1 + 0xc) = iVar2;
    iVar5 = FUN_10001864();
    iVar3 = DAT_10002264;
    *puVar1 = 3;
    puVar1[1] = (char)_DAT_d0000000;
    *(undefined4 *)(puVar1 + 8) = DAT_10002268;
    *(int *)(puVar1 + 0x10) = (iVar5 + iVar3) * 4;
    *(undefined2 *)(puVar1 + 2) = 0xf;
    puVar6 = *(undefined2 **)(puVar1 + 0x14);
    *(undefined2 *)(puVar1 + 4) = 0xffff;
    *puVar6 = 0xffff;
    puVar6[0x18] = 1;
    puVar6[0xc] = 0;
    puVar6[0x24] = 2;
    puVar6[0x30] = 3;
    puVar6[0x3c] = 4;
    puVar6[0x48] = 5;
    puVar6[0x54] = 6;
    puVar6[0x60] = 7;
    puVar6[0x6c] = 8;
    puVar6[0x78] = 9;
    puVar6[0x84] = 10;
    puVar6[0x90] = 0xb;
    puVar6[0x9c] = 0xc;
    puVar6[0xa8] = 0xd;
    puVar6[0xb4] = 0xe;
    uVar4 = DAT_10002270;
    *(undefined1 **)(DAT_1000226c + 0xc) = puVar1;
    *(undefined4 *)(iVar2 + 0x20) = 8;
    FUN_10001904(3,uVar4);
    FUN_100018e0(3,1);
    *DAT_10002274 = 8;
  }
  FUN_10001dd4(DAT_1000225c,10);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10002278 @ 10002278  (170 bytes) */
/* ---------------------------------------------------------------- */

undefined4
FUN_10002278(sbyte *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,undefined4 param_6)

{
  short sVar1;
  bool bVar2;
  undefined4 uVar3;
  ushort uVar4;
  undefined2 *puVar5;
  uint uVar6;
  
  uVar6 = 0;
  bVar2 = (bool)isCurrentModePrivileged();
  if (bVar2) {
    uVar6 = isIRQinterruptsEnabled();
  }
  disableIRQinterrupts();
  do {
  } while (**(int **)(param_1 + 0x10) == 0);
  DataMemoryBarrier(0x1f);
  sVar1 = *(short *)(param_1 + 2);
  puVar5 = (undefined2 *)(*(int *)(param_1 + 0x14) + sVar1 * 0x18);
  if (sVar1 < 0) {
    DataMemoryBarrier(0x1f);
    **(undefined4 **)(param_1 + 0x10) = 0;
    bVar2 = (bool)isCurrentModePrivileged();
    if (bVar2) {
      enableIRQinterrupts((uVar6 & 1) == 1);
    }
    uVar3 = 0xffffffff;
  }
  else {
    *(undefined2 *)(param_1 + 2) = *puVar5;
    DataMemoryBarrier(0x1f);
    **(undefined4 **)(param_1 + 0x10) = 0;
    bVar2 = (bool)isCurrentModePrivileged();
    if (bVar2) {
      enableIRQinterrupts((uVar6 & 1) == 1);
    }
    *(undefined4 *)(puVar5 + 6) = param_4;
    *(undefined4 *)(puVar5 + 8) = param_5;
    uVar4 = puVar5[1] + 1 & 0x7fff;
    *(undefined4 *)(puVar5 + 4) = param_3;
    *(undefined4 *)(puVar5 + 10) = param_6;
    if (uVar4 == 0) {
      uVar4 = 1;
    }
    puVar5[1] = uVar4;
    uVar3 = CONCAT22(sVar1,uVar4);
    uVar6 = 0;
    bVar2 = (bool)isCurrentModePrivileged();
    if (bVar2) {
      uVar6 = isIRQinterruptsEnabled();
    }
    disableIRQinterrupts();
    do {
    } while (**(int **)(param_1 + 0x10) == 0);
    DataMemoryBarrier(0x1f);
    *puVar5 = *(undefined2 *)(param_1 + 4);
    *(short *)(param_1 + 4) = sVar1;
    DataMemoryBarrier(0x1f);
    **(undefined4 **)(param_1 + 0x10) = 0;
    bVar2 = (bool)isCurrentModePrivileged();
    if (bVar2) {
      enableIRQinterrupts((uVar6 & 1) == 1);
    }
    *(int *)(*(int *)(param_1 + 0xc) + DAT_10002324) = 1 << *param_1;
  }
  return uVar3;
}



/* ---------------------------------------------------------------- */
/* FUN_10002328 @ 10002328  (112 bytes) */
/* ---------------------------------------------------------------- */

undefined4 FUN_10002328(sbyte *param_1,uint param_2)

{
  ushort uVar1;
  bool bVar2;
  int iVar3;
  uint uVar4;
  
  if ((int)param_2 >> 0x10 < (int)(uint)*(ushort *)(param_1 + 10)) {
    iVar3 = *(int *)(param_1 + 0x14) + ((int)param_2 >> 0x10) * 0x18;
    uVar4 = 0;
    bVar2 = (bool)isCurrentModePrivileged();
    if (bVar2) {
      uVar4 = isIRQinterruptsEnabled();
    }
    disableIRQinterrupts();
    do {
    } while (**(int **)(param_1 + 0x10) == 0);
    DataMemoryBarrier(0x1f);
    uVar1 = *(ushort *)(iVar3 + 2);
    if ((uint)uVar1 == (param_2 & 0xffff)) {
      *(ushort *)(iVar3 + 2) = uVar1 | 0x8000;
      param_1[6] = 1;
      DataMemoryBarrier(0x1f);
      **(undefined4 **)(param_1 + 0x10) = 0;
      bVar2 = (bool)isCurrentModePrivileged();
      if (bVar2) {
        enableIRQinterrupts((uVar4 & 1) == 1);
      }
      *(int *)(*(int *)(param_1 + 0xc) + DAT_10002398) = 1 << *param_1;
      return 1;
    }
    DataMemoryBarrier(0x1f);
    **(undefined4 **)(param_1 + 0x10) = 0;
    bVar2 = (bool)isCurrentModePrivileged();
    if (bVar2) {
      enableIRQinterrupts((uVar4 & 1) == 1);
    }
  }
  return 0;
}



/* ---------------------------------------------------------------- */
/* FUN_1000239c @ 1000239c  (232 bytes) */
/* ---------------------------------------------------------------- */

/* WARNING: Removing unreachable block (ram,0x1000247e) */

void FUN_1000239c(undefined4 param_1)

{
  bool bVar1;
  uint uVar2;
  undefined4 *puVar3;
  int iVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  longlong lVar9;
  longlong lVar10;
  longlong lVar11;
  undefined8 uVar12;
  
  lVar9 = FUN_100025c8();
  lVar10 = FUN_10002c98(param_1,0,1000,0);
  lVar9 = lVar9 + lVar10;
  if (lVar9 < 0) {
    lVar9 = CONCAT44(DAT_10002484,0xffffffff);
  }
  uVar6 = (uint)((ulonglong)lVar9 >> 0x20);
  lVar10 = lVar9 + -6;
  uVar7 = (uint)((ulonglong)lVar10 >> 0x20);
  if ((uVar6 <= uVar7 && uVar7 != uVar6) || ((uVar7 == uVar6 && ((uint)lVar9 < (uint)lVar10)))) {
    lVar10 = 0;
    uVar12 = FUN_100025c8();
    lVar11 = CONCAT44(-(uint)((int)uVar12 != 0) - (int)((ulonglong)uVar12 >> 0x20),-(int)uVar12);
  }
  else {
    lVar11 = FUN_100025c8();
    lVar11 = lVar10 - lVar11;
  }
  uVar2 = (uint)((ulonglong)lVar10 >> 0x20);
  uVar7 = (uint)lVar10;
  if (0 < lVar11) {
    uVar12 = FUN_100025c8();
    iVar5 = (int)((ulonglong)uVar12 >> 0x20);
    puVar3 = DAT_10002494;
    iVar4 = DAT_10002490;
    if (((int)((uVar2 - iVar5) - (uint)(uVar7 < (uint)uVar12)) < 0) ||
       (iVar5 = FUN_10002278(DAT_1000248c,iVar5,uVar7,uVar2,DAT_10002488,0), puVar3 = DAT_10002494,
       iVar4 = DAT_10002490, iVar5 != -1)) {
      while ((*(uint *)(iVar4 + 0x24) < uVar2 ||
             ((*(uint *)(iVar4 + 0x28) < uVar7 && (uVar2 == *(uint *)(iVar4 + 0x24)))))) {
        uVar8 = 0;
        bVar1 = (bool)isCurrentModePrivileged();
        if (bVar1) {
          uVar8 = isIRQinterruptsEnabled();
        }
        disableIRQinterrupts();
        do {
        } while (*(int *)*puVar3 == 0);
        DataMemoryBarrier(0x1f);
        DataMemoryBarrier(0x1f);
        *(undefined4 *)*puVar3 = 0;
        bVar1 = (bool)isCurrentModePrivileged();
        if (bVar1) {
          enableIRQinterrupts((uVar8 & 1) == 1);
        }
        WaitForEvent();
      }
    }
  }
  FUN_100025dc((uint)lVar9,uVar6);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_100025a4 @ 100025a4  (12 bytes) */
/* ---------------------------------------------------------------- */

void FUN_100025a4(undefined4 param_1,undefined4 param_2)

{
  FUN_10001760(DAT_100025b4,param_2,DAT_100025b0);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_100025b8 @ 100025b8  (16 bytes) */
/* ---------------------------------------------------------------- */

undefined4 FUN_100025b8(int param_1)

{
  int iVar1;
  bool bVar2;
  
  iVar1 = *(int *)(param_1 + 0x24);
  do {
    bVar2 = iVar1 != *(int *)(param_1 + 0x24);
    iVar1 = *(int *)(param_1 + 0x24);
  } while (bVar2);
  return *(undefined4 *)(param_1 + 0x28);
}



/* ---------------------------------------------------------------- */
/* FUN_100025c8 @ 100025c8  (16 bytes) */
/* ---------------------------------------------------------------- */

undefined4 FUN_100025c8(void)

{
  int iVar1;
  bool bVar2;
  
  iVar1 = *(int *)(DAT_100025d8 + 0x24);
  do {
    bVar2 = *(int *)(DAT_100025d8 + 0x24) != iVar1;
    iVar1 = *(int *)(DAT_100025d8 + 0x24);
  } while (bVar2);
  return *(undefined4 *)(DAT_100025d8 + 0x28);
}



/* ---------------------------------------------------------------- */
/* FUN_100025dc @ 100025dc  (28 bytes) */
/* ---------------------------------------------------------------- */

void FUN_100025dc(uint param_1,uint param_2)

{
  uint uVar1;
  
  do {
    uVar1 = *(uint *)(DAT_100025f8 + 0x24);
  } while (uVar1 < param_2);
  while ((param_2 == uVar1 && (*(uint *)(DAT_100025f8 + 0x28) < param_1))) {
    uVar1 = *(uint *)(DAT_100025f8 + 0x24);
  }
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_100025fc @ 100025fc  (234 bytes) */
/* ---------------------------------------------------------------- */

void FUN_100025fc(int param_1,uint param_2,int param_3,undefined4 param_4,int param_5)

{
  undefined4 uVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  uint *puVar5;
  uint uVar6;
  int iVar7;
  bool bVar8;
  
  uVar1 = FUN_10002b7c(param_4,param_5);
  uVar6 = param_5 * 0x100;
  puVar5 = (uint *)(param_1 * 0xc + DAT_100026e8);
  if (puVar5[1] <= uVar6 && uVar6 - puVar5[1] != 0) {
    puVar5[1] = uVar6;
  }
  uVar3 = param_1 - 4U & 0xff;
  if ((uVar3 < 2) && (param_2 == 1)) {
    puVar5[0xc00] = 3;
    iVar7 = DAT_100026ec;
    do {
    } while ((puVar5[2] & 1) == 0);
    param_1 = param_1 << 2;
    puVar5[0x400] = (param_3 << 5 ^ *puVar5) & 0xe0;
  }
  else {
    puVar5[0xc00] = 0x800;
    iVar7 = DAT_100026ec;
    param_1 = param_1 * 4;
    if (*(int *)(DAT_100026ec + param_1) != 0) {
      iVar2 = FUN_10002b7c(*(undefined4 *)(DAT_100026ec + 0x14));
      uVar4 = (iVar2 + 1) * 3;
      do {
        bVar8 = 2 < uVar4;
        uVar4 = uVar4 - 3;
      } while (bVar8);
    }
    puVar5[0x400] = (param_3 << 5 ^ *puVar5) & 0xe0;
    if (1 < uVar3) goto LAB_1000266a;
  }
  puVar5[0x400] = (*puVar5 ^ param_2) & 3;
  do {
  } while ((1 << (param_2 & 0xff) & puVar5[2]) == 0);
LAB_1000266a:
  puVar5[0x800] = 0x800;
  puVar5[1] = uVar6;
  *(undefined4 *)(iVar7 + param_1) = uVar1;
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_100026f0 @ 100026f0  (222 bytes) */
/* ---------------------------------------------------------------- */

void FUN_100026f0(int param_1,uint param_2,int param_3,undefined4 param_4)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  uint *puVar4;
  int iVar5;
  bool bVar6;
  
  puVar4 = (uint *)(param_1 * 0xc + DAT_100027d0);
  if (puVar4[1] < 0x100) {
    puVar4[1] = 0x100;
  }
  uVar2 = param_1 - 4U & 0xff;
  if ((uVar2 < 2) && (param_2 == 1)) {
    puVar4[0xc00] = 3;
    iVar5 = DAT_100027d4;
    do {
    } while ((puVar4[2] & 1) == 0);
    param_1 = param_1 << 2;
    puVar4[0x400] = (param_3 << 5 ^ *puVar4) & 0xe0;
  }
  else {
    puVar4[0xc00] = 0x800;
    iVar5 = DAT_100027d4;
    param_1 = param_1 * 4;
    if (*(int *)(DAT_100027d4 + param_1) != 0) {
      iVar1 = FUN_10002b7c(*(undefined4 *)(DAT_100027d4 + 0x14));
      uVar3 = (iVar1 + 1) * 3;
      do {
        bVar6 = 2 < uVar3;
        uVar3 = uVar3 - 3;
      } while (bVar6);
    }
    puVar4[0x400] = (param_3 << 5 ^ *puVar4) & 0xe0;
    if (1 < uVar2) goto LAB_10002752;
  }
  puVar4[0x400] = (*puVar4 ^ param_2) & 3;
  do {
  } while ((1 << (param_2 & 0xff) & puVar4[2]) == 0);
LAB_10002752:
  puVar4[0x800] = 0x800;
  puVar4[1] = 0x100;
  *(undefined4 *)(iVar5 + param_1) = param_4;
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_100027d8 @ 100027d8  (8 bytes) */
/* ---------------------------------------------------------------- */

undefined4 FUN_100027d8(int param_1)

{
  return *(undefined4 *)(param_1 * 4 + DAT_100027e0);
}



/* ---------------------------------------------------------------- */
/* FUN_100027e4 @ 100027e4  (138 bytes) */
/* ---------------------------------------------------------------- */

void FUN_100027e4(uint *param_1,uint param_2,undefined4 param_3,int param_4,int param_5)

{
  uint *puVar1;
  int iVar2;
  undefined4 uVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  
  uVar3 = FUN_10002b7c(DAT_10002870);
  uVar4 = FUN_10002b7c(param_3,uVar3);
  puVar1 = DAT_10002878;
  uVar6 = param_4 << 0x10 | param_5 << 0xc;
  if ((((-1 < (int)*param_1) || ((*param_1 & 0x3f) != param_2)) || ((param_1[2] & 0xfff) != uVar4))
     || ((param_1[3] & 0x77000) != uVar6)) {
    uVar5 = (-(uint)((int)param_1 + DAT_10002874 != 0) & 0xfffff000) + 0x2000;
    *DAT_1000287c = uVar5;
    *DAT_10002880 = uVar5;
    iVar2 = DAT_10002884;
    do {
    } while ((uVar5 & ~*puVar1) != 0);
    *param_1 = param_2;
    param_1[2] = uVar4;
    *(undefined4 *)((int)param_1 + iVar2) = 0x21;
    do {
    } while (-1 < (int)*param_1);
    param_1[3] = uVar6;
    *(undefined4 *)((int)param_1 + iVar2) = 8;
  }
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10002888 @ 10002888  (12 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10002888(undefined4 param_1,uint param_2)

{
  *(uint *)(DAT_10002894 + 0x2c) = param_2 | 0x200;
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10002898 @ 10002898  (14 bytes) */
/* ---------------------------------------------------------------- */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_10002898(undefined4 param_1)

{
  (*(code *)(uint)_UsageFault)(_BusFault,param_1);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_100028a8 @ 100028a8  (16 bytes) */
/* ---------------------------------------------------------------- */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_100028a8(undefined4 param_1)

{
  (*(code *)(uint)_UsageFault)(_DAT_00000016,param_1);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_100028b8 @ 100028b8  (66 bytes) */
/* ---------------------------------------------------------------- */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

byte FUN_100028b8(int *param_1,int param_2)

{
  int iVar1;
  int iVar2;
  byte bVar3;
  
  if (param_2 == 0) {
    bVar3 = 1;
  }
  else {
    iVar2 = 0;
    bVar3 = 1;
    do {
      iVar1 = (*(code *)(uint)_UsageFault)(_BusFault,*param_1);
      *param_1 = iVar1;
      param_1 = param_1 + 1;
      iVar2 = iVar2 + 1;
      bVar3 = bVar3 & iVar1 != 0;
    } while (param_2 != iVar2);
  }
  return bVar3;
}



/* ---------------------------------------------------------------- */
/* FUN_100028fc @ 100028fc  (28 bytes) */
/* ---------------------------------------------------------------- */

void FUN_100028fc(void)

{
  undefined4 *puVar1;
  
  puVar1 = DAT_10002918;
  *DAT_10002918 = 0xaa0;
  puVar1[3] = 0x11a;
  *DAT_10002920 = DAT_1000291c;
  do {
  } while (-1 < (int)puVar1[1]);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10002928 @ 10002928  (8 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10002928(void)

{
                    /* WARNING: Subroutine does not return */
  FUN_10001704(DAT_10002930);
}



/* ---------------------------------------------------------------- */
/* FUN_10002934 @ 10002934  (32 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10002934(void)

{
  code *pcVar1;
  uint uVar2;
  undefined4 *puVar3;
  uint uVar4;
  bool bVar5;
  
  if (DAT_10002954 < DAT_10002958) {
    uVar4 = (int)DAT_10002958 + (-1 - (int)DAT_10002954);
    uVar2 = 0;
    puVar3 = DAT_10002954;
    do {
      pcVar1 = (code *)*puVar3;
      puVar3 = puVar3 + 1;
      (*pcVar1)();
      bVar5 = uVar2 != uVar4 >> 2;
      uVar2 = uVar2 + 1;
    } while (bVar5);
  }
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_1000295c @ 1000295c  (24 bytes) */
/* ---------------------------------------------------------------- */

void FUN_1000295c(void)

{
  uint *puVar1;
  uint uVar2;
  
  puVar1 = DAT_1000297c;
  *DAT_10002974 = DAT_10002978;
  uVar2 = DAT_10002980;
  *DAT_10002984 = DAT_10002980;
  do {
  } while ((uVar2 & ~*puVar1) != 0);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10002988 @ 10002988  (18 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10002988(void)

{
  if (*(int *)(DAT_1000299c + 0x4c) == 0) {
    *DAT_100029a0 = 0x40000;
  }
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10002b7c @ 10002b7c  (32 bytes) */
/* ---------------------------------------------------------------- */

ulonglong FUN_10002b7c(int param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  uint uVar3;
  int extraout_r2;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  ulonglong uVar7;
  
  iVar1 = DAT_10002bc4;
  if ((*(uint *)(DAT_10002bc4 + 0x78) >> 1 & 1) != 0) {
    uVar2 = *(undefined4 *)(DAT_10002bc4 + 0x60);
    uVar4 = *(undefined4 *)(DAT_10002bc4 + 100);
    uVar6 = *(undefined4 *)(DAT_10002bc4 + 0x74);
    uVar5 = *(undefined4 *)(DAT_10002bc4 + 0x70);
    uVar7 = FUN_10002b84();
    *(undefined4 *)(extraout_r2 + 0x60) = uVar2;
    *(undefined4 *)(extraout_r2 + 100) = uVar4;
    *(undefined4 *)(extraout_r2 + 0x74) = uVar6;
    *(undefined4 *)(extraout_r2 + 0x70) = uVar5;
    return uVar7;
  }
  *(int *)(DAT_10002bc4 + 0x60) = param_1;
  *(int *)(iVar1 + 100) = param_2;
  if (param_2 != 0) {
    return *(ulonglong *)(iVar1 + 0x70);
  }
  uVar2 = 0;
  if (param_1 != 0) {
    uVar2 = 0xffffffff;
  }
  uVar3 = FUN_10005288(uVar2);
  return (ulonglong)uVar3;
}



/* ---------------------------------------------------------------- */
/* FUN_10002b84 @ 10002b84  (38 bytes) */
/* ---------------------------------------------------------------- */

ulonglong FUN_10002b84(int param_1,int param_2,int param_3)

{
  undefined4 uVar1;
  uint uVar2;
  
  *(int *)(param_3 + 0x60) = param_1;
  *(int *)(param_3 + 100) = param_2;
  if (param_2 != 0) {
    return *(ulonglong *)(param_3 + 0x70);
  }
  uVar1 = 0;
  if (param_1 != 0) {
    uVar1 = 0xffffffff;
  }
  uVar2 = FUN_10005288(uVar1);
  return (ulonglong)uVar2;
}



/* ---------------------------------------------------------------- */
/* FUN_10002c98 @ 10002c98  (50 bytes) */
/* ---------------------------------------------------------------- */

undefined8 FUN_10002c98(uint param_1,int param_2,uint param_3,int param_4)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  
  uVar4 = (param_3 >> 0x10) * (param_1 & 0xffff);
  uVar1 = (param_3 & 0xffff) * (param_1 & 0xffff);
  uVar3 = (param_1 >> 0x10) * (param_3 & 0xffff);
  uVar6 = uVar4 * 0x10000;
  uVar2 = uVar1 + uVar6;
  uVar5 = uVar3 * 0x10000;
  return CONCAT44((param_1 >> 0x10) * (param_3 >> 0x10) + (uVar4 >> 0x10) +
                  (uint)CARRY4(uVar1,uVar6) + (uVar3 >> 0x10) + (uint)CARRY4(uVar2,uVar5) +
                  param_3 * param_2 + param_1 * param_4,uVar2 + uVar5);
}



/* ---------------------------------------------------------------- */
/* FUN_10002cd2 @ 10002cd2  (6 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10002cd2(void)

{
                    /* WARNING: Could not recover jumptable at 0x10002cd6. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(DAT_10002d0c + 4))();
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10002cd8 @ 10002cd8  (6 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10002cd8(void)

{
                    /* WARNING: Could not recover jumptable at 0x10002cdc. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)*DAT_10002d0c)();
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10002cde @ 10002cde  (34 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10002cde(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  
  if ((*(uint *)(DAT_10002d10 + 0x78) >> 1 & 1) == 0) {
                    /* WARNING: Could not recover jumptable at 0x10002cea. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (**(code **)(DAT_10002d0c + 0xc))();
    return;
  }
  uVar2 = *(undefined4 *)(DAT_10002d10 + 0x60);
  uVar3 = *(undefined4 *)(DAT_10002d10 + 100);
  uVar5 = *(undefined4 *)(DAT_10002d10 + 0x74);
  uVar4 = *(undefined4 *)(DAT_10002d10 + 0x70);
  FUN_10002ce6();
  iVar1 = DAT_10002d10;
  *(undefined4 *)(DAT_10002d10 + 0x60) = uVar2;
  *(undefined4 *)(iVar1 + 100) = uVar3;
  *(undefined4 *)(iVar1 + 0x74) = uVar5;
  *(undefined4 *)(iVar1 + 0x70) = uVar4;
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10002ce6 @ 10002ce6  (6 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10002ce6(void)

{
                    /* WARNING: Could not recover jumptable at 0x10002cea. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(DAT_10002d0c + 0xc))();
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10002d06 @ 10002d06  (6 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10002d06(void)

{
                    /* WARNING: Could not recover jumptable at 0x10002d0a. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(DAT_10002d0c + 8))();
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10002d14 @ 10002d14  (58 bytes) */
/* ---------------------------------------------------------------- */

undefined4 FUN_10002d14(int param_1)

{
  uint uVar1;
  undefined4 uVar2;
  int iVar3;
  uint uVar4;
  code *UNRECOVERED_JUMPTABLE;
  
  if (param_1 == 0) {
    return 0;
  }
  uVar1 = (*(code *)*DAT_10002d60)();
  uVar4 = param_1 << (uVar1 & 0xff);
  iVar3 = -(uVar1 - 0x9e);
  uVar1 = uVar4 + 0x80;
  if (uVar4 < 0xffffff80) {
    if ((uVar1 & 0xff) == 0) {
      uVar1 = (uVar1 >> 9) << 10;
    }
    else {
      uVar1 = uVar1 * 2;
    }
  }
  else {
    iVar3 = iVar3 + 1;
  }
                    /* WARNING: Could not recover jumptable at 0x10002d50. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  uVar2 = (*UNRECOVERED_JUMPTABLE)(iVar3 << 0x17 | uVar1 >> 9);
  return uVar2;
}



/* ---------------------------------------------------------------- */
/* FUN_10002d64 @ 10002d64  (6 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10002d64(void)

{
                    /* WARNING: Could not recover jumptable at 0x10002d68. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(DAT_10002d6c + 0x24))();
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10002e10 @ 10002e10  (6 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10002e10(void)

{
                    /* WARNING: Could not recover jumptable at 0x10002e14. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)*DAT_10002e18)();
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10002e1c @ 10002e1c  (6 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10002e1c(void)

{
                    /* WARNING: Could not recover jumptable at 0x10002e20. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(DAT_10002e24 + 4))();
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10002e38 @ 10002e38  (40 bytes) */
/* ---------------------------------------------------------------- */

bool FUN_10002e38(undefined4 param_1,undefined4 param_2)

{
  code *pcVar1;
  undefined1 auStack_c [4];
  
  pcVar1 = (code *)*DAT_10002e60;
  if (pcVar1 == (code *)0x0) {
    FUN_100030f4();
  }
  else {
    (*pcVar1)(DAT_10002e64,auStack_c,0xffffffff,param_1,param_2);
  }
  return pcVar1 != (code *)0x0;
}



/* ---------------------------------------------------------------- */
/* FUN_10002e68 @ 10002e68  (4 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10002e68(void)

{
  do {
    software_bkpt(0);
  } while( true );
}



/* ---------------------------------------------------------------- */
/* FUN_10002e6c @ 10002e6c  (8 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10002e6c(void)

{
  code *pcVar1;
  uint uVar2;
  undefined4 *puVar3;
  uint uVar4;
  bool bVar5;
  
  FUN_10002e68();
  FUN_10002934();
  if (DAT_10002e98 < DAT_10002e9c) {
    uVar4 = (int)DAT_10002e9c + (-1 - (int)DAT_10002e98);
    uVar2 = 0;
    puVar3 = DAT_10002e98;
    do {
      pcVar1 = (code *)*puVar3;
      puVar3 = puVar3 + 1;
      (*pcVar1)();
      bVar5 = uVar2 != uVar4 >> 2;
      uVar2 = uVar2 + 1;
    } while (bVar5);
  }
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10002e74 @ 10002e74  (36 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10002e74(void)

{
  code *pcVar1;
  uint uVar2;
  undefined4 *puVar3;
  uint uVar4;
  bool bVar5;
  
  FUN_10002934();
  if (DAT_10002e98 < DAT_10002e9c) {
    uVar4 = (int)DAT_10002e9c + (-1 - (int)DAT_10002e98);
    uVar2 = 0;
    puVar3 = DAT_10002e98;
    do {
      pcVar1 = (code *)*puVar3;
      puVar3 = puVar3 + 1;
      (*pcVar1)();
      bVar5 = uVar2 != uVar4 >> 2;
      uVar2 = uVar2 + 1;
    } while (bVar5);
  }
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10002eb0 @ 10002eb0  (170 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10002eb0(undefined4 *param_1,char *param_2,int param_3)

{
  int iVar1;
  int iVar2;
  int iVar3;
  
  if (*(char *)((int)param_1 + 0x15) == '\0') {
    (*(code *)*param_1)(param_2,param_3);
    return;
  }
  if (param_3 < 1) {
    return;
  }
  if ((*param_2 == '\n') && (*(char *)(param_1 + 5) == '\0')) {
    (*(code *)*param_1)(DAT_10002f5c,2);
    if (param_3 != 1) {
      iVar3 = 1;
LAB_10002ed4:
      iVar1 = 1;
      do {
        iVar2 = iVar1 + 1;
        if ((param_2[iVar1] == '\n') && (param_2[iVar1 + -1] != '\r')) {
          if (iVar3 < iVar1) {
            (*(code *)*param_1)(param_2 + iVar3,iVar1 - iVar3);
          }
          (*(code *)*param_1)(DAT_10002f5c,2);
          iVar3 = iVar2;
        }
        iVar1 = iVar2;
      } while (iVar2 != param_3);
      if (iVar3 < param_3) goto LAB_10002efc;
    }
    *(bool *)(param_1 + 5) = param_2[param_3 + -1] == '\r';
  }
  else {
    iVar3 = 0;
    if (param_3 != 1) goto LAB_10002ed4;
LAB_10002efc:
    (*(code *)*param_1)(param_2 + iVar3,param_3 - iVar3);
    *(bool *)(param_1 + 5) = param_2[param_3 + -1] == '\r';
  }
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10002f60 @ 10002f60  (222 bytes) */
/* ---------------------------------------------------------------- */

int FUN_10002f60(undefined4 param_1,int param_2,int param_3,int param_4)

{
  undefined4 uVar1;
  undefined4 *puVar2;
  int iVar3;
  int *piVar4;
  int *piVar5;
  code *pcVar6;
  longlong lVar7;
  undefined1 local_29 [5];
  
  lVar7 = FUN_100025c8();
  uVar1 = DAT_10003048;
  lVar7 = lVar7 + (ulonglong)DAT_10003040;
  if (lVar7 < 0) {
    lVar7 = CONCAT44(DAT_10003044,0xffffffff);
  }
  iVar3 = FUN_100052c8(DAT_10003048,DAT_10003048,(int)lVar7,(int)((ulonglong)lVar7 >> 0x20));
  if (param_2 == -1) {
    param_2 = FUN_100051c8(param_1);
  }
  puVar2 = DAT_10003054;
  pcVar6 = DAT_10003058;
  if (param_4 != 0) {
    pcVar6 = DAT_1000304c;
  }
  piVar5 = (int *)*DAT_10003050;
  if (piVar5 != (int *)0x0) {
    if (param_3 == 0) {
      do {
        while ((*piVar5 == 0 ||
               ((piVar4 = (int *)*puVar2, piVar4 != (int *)0x0 && (piVar5 != piVar4))))) {
          piVar5 = (int *)piVar5[4];
          if (piVar5 == (int *)0x0) goto LAB_10002fe4;
        }
        (*pcVar6)(piVar5,param_1,param_2);
        piVar5 = (int *)piVar5[4];
      } while (piVar5 != (int *)0x0);
    }
    else {
      do {
        if ((*piVar5 != 0) &&
           ((piVar4 = (int *)*puVar2, piVar4 == (int *)0x0 || (piVar4 == piVar5)))) {
          (*pcVar6)(piVar5,param_1,param_2);
          local_29[0] = 10;
          (*pcVar6)(piVar5,local_29,1);
        }
        piVar5 = (int *)piVar5[4];
      } while (piVar5 != (int *)0x0);
    }
  }
LAB_10002fe4:
  if (iVar3 != 0) {
    FUN_10005248(uVar1);
  }
  return param_2;
}



/* ---------------------------------------------------------------- */
/* FUN_1000305c @ 1000305c  (4 bytes) */
/* ---------------------------------------------------------------- */

undefined4 FUN_1000305c(void)

{
  return 0;
}



/* ---------------------------------------------------------------- */
/* FUN_10003060 @ 10003060  (128 bytes) */
/* ---------------------------------------------------------------- */

undefined4 FUN_10003060(undefined4 param_1)

{
  undefined4 uVar1;
  undefined4 *puVar2;
  int iVar3;
  int *piVar4;
  longlong lVar5;
  undefined1 local_21 [5];
  
  local_21[0] = (undefined1)param_1;
  lVar5 = FUN_100025c8();
  uVar1 = DAT_100030e8;
  lVar5 = lVar5 + (ulonglong)DAT_100030e0;
  if (lVar5 < 0) {
    lVar5 = CONCAT44(DAT_100030e4,0xffffffff);
  }
  iVar3 = FUN_100052c8(DAT_100030e8,DAT_100030e8,(int)lVar5,(int)((ulonglong)lVar5 >> 0x20));
  puVar2 = DAT_100030f0;
  for (piVar4 = (int *)*DAT_100030ec; piVar4 != (int *)0x0; piVar4 = (int *)piVar4[4]) {
    while ((*piVar4 == 0 || (((int *)*puVar2 != (int *)0x0 && (piVar4 != (int *)*puVar2))))) {
      piVar4 = (int *)piVar4[4];
      if (piVar4 == (int *)0x0) goto LAB_100030c6;
    }
    FUN_10002eb0(piVar4,local_21,1);
  }
LAB_100030c6:
  if (iVar3 != 0) {
    FUN_10005248(uVar1);
  }
  return param_1;
}



/* ---------------------------------------------------------------- */
/* FUN_100030f4 @ 100030f4  (56 bytes) */
/* ---------------------------------------------------------------- */

undefined4 FUN_100030f4(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  
  uVar1 = FUN_100051c8();
  FUN_10002f60(param_1,uVar1,1,1);
  iVar2 = *DAT_1000312c;
  do {
    if (iVar2 == 0) {
      return uVar1;
    }
    while (*(code **)(iVar2 + 4) == (code *)0x0) {
      iVar2 = *(int *)(iVar2 + 0x10);
      if (iVar2 == 0) {
        return uVar1;
      }
    }
    (**(code **)(iVar2 + 4))();
    iVar2 = *(int *)(iVar2 + 0x10);
  } while( true );
}



/* ---------------------------------------------------------------- */
/* FUN_10003130 @ 10003130  (58 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10003130(undefined4 param_1)

{
  undefined1 local_28 [16];
  undefined1 auStack_18 [5];
  undefined1 auStack_13 [11];
  
  FUN_10002e10(local_28,0,0xd);
  FUN_10002e10(auStack_18,0,0xd);
  local_28[0] = 0x4b;
  FUN_10005238(local_28,auStack_18,0xd);
  FUN_10002e1c(param_1,auStack_13,8);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_1000316c @ 1000316c  (172 bytes) */
/* ---------------------------------------------------------------- */

undefined4 FUN_1000316c(int param_1,int param_2,uint param_3,uint param_4,char param_5)

{
  uint uVar1;
  undefined4 uVar2;
  int iVar3;
  int iVar4;
  
  if (param_3 < 0x20) {
    iVar4 = param_1 + param_2 * 0x18;
    uVar1 = *(uint *)(iVar4 + 0xdc);
    uVar2 = *(undefined4 *)(iVar4 + 0xcc);
    *(undefined4 *)(iVar4 + DAT_10003218) = 0x20000;
    iVar3 = (-(uint)(param_5 == '\0') & 0xffffffe1) + DAT_1000321c;
    if (5 < param_4) {
      do {
        param_4 = param_4 - 5;
        *(uint *)(iVar4 + 0xdc) = param_3 << 5 | 0x14000000;
        param_3 = param_3 + 5 & 0x1f;
        *(int *)(iVar4 + 0xd8) = iVar3;
      } while (5 < param_4);
    }
    param_1 = param_2 * 0x18 + param_1;
    *(uint *)(param_1 + 0xdc) = param_4 << 0x1a | param_3 << 5;
    *(int *)(param_1 + 0xd8) = iVar3;
    *(uint *)(param_1 + 0xdc) = uVar1;
    *(undefined4 *)(param_1 + 0xcc) = uVar2;
    uVar2 = 0;
  }
  else {
    uVar2 = 0xfffffffb;
  }
  return uVar2;
}



/* ---------------------------------------------------------------- */
/* FUN_10003220 @ 10003220  (148 bytes) */
/* ---------------------------------------------------------------- */

undefined4 FUN_10003220(uint *param_1,uint param_2,uint param_3,uint *param_4)

{
  uint uVar1;
  undefined4 *puVar2;
  
  *param_1 = *param_1 & ~(1 << (param_2 & 0xff));
  if (param_4 == (uint *)0x0) {
    param_1[param_2 * 6 + 0x32] = 0x10000;
    param_1[param_2 * 6 + 0x34] = 0xc0000;
    param_1[param_2 * 6 + 0x33] = 0x1f000;
    param_1[param_2 * 6 + 0x37] = 0;
  }
  else {
    param_1[param_2 * 6 + 0x32] = *param_4;
    param_1[param_2 * 6 + 0x34] = param_4[2];
    uVar1 = param_4[3];
    param_1[param_2 * 6 + 0x33] = param_4[1];
    param_1[param_2 * 6 + 0x37] = uVar1;
  }
  puVar2 = (undefined4 *)((int)param_1 + DAT_100032b4 + param_2 * 0x18);
  *puVar2 = 0x80000000;
  *puVar2 = 0x80000000;
  param_1[2] = DAT_100032b8 << (param_2 & 0xff);
  param_1[0x800] = 1 << (param_2 + 4 & 0xff);
  param_1[0x800] = 1 << (param_2 + 8 & 0xff);
  param_1[param_2 * 6 + 0x36] = param_3 & 0x1f;
  return 0;
}



/* ---------------------------------------------------------------- */
/* FUN_100032bc @ 100032bc  (358 bytes) */
/* ---------------------------------------------------------------- */

undefined4 FUN_100032bc(int *param_1,int *param_2,int *param_3,uint *param_4)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  uint uVar4;
  int iVar5;
  uint uVar6;
  int iVar7;
  uint uVar8;
  int iVar9;
  uint uVar10;
  int iVar11;
  uint uVar12;
  int iVar13;
  uint uVar14;
  int iVar15;
  int iVar16;
  bool bVar17;
  
  uVar2 = DAT_10003428;
  uVar1 = DAT_10003424;
  iVar13 = 2;
  iVar5 = 2;
  do {
    iVar15 = DAT_10003430;
    uVar6 = iVar5 + DAT_1000342c;
    *param_2 = uVar6 * 0x100000;
    iVar5 = (uVar6 & 0xfff) * 4;
    iVar15 = iVar5 + iVar15;
    iVar5 = FUN_100017a4(uVar1,0,iVar15,iVar5 + DAT_10003434,uVar2);
    if ((iVar15 <= iVar5) &&
       (iVar16 = (int)(char)(iVar5 - iVar15), -1 < (iVar5 - iVar15) * 0x1000000)) {
      uVar3 = FUN_10001734();
      iVar5 = DAT_1000343c;
      iVar15 = *param_2;
      iVar7 = ((uint)(DAT_10003438 + iVar15) >> 0x14) * 4;
      uVar6 = (uint)(char)*(byte *)((int)param_1 + 5);
      uVar8 = *(uint *)(DAT_1000343c + iVar7);
      uVar4 = (uint)*(byte *)(param_1 + 1);
      iVar9 = (1 << uVar4) + -1;
      uVar10 = 0x20 - uVar4;
      if ((int)uVar6 < 0) {
        uVar6 = uVar10;
        if (-1 < (int)uVar10) {
          do {
            if ((uVar8 & iVar9 << (uVar6 & 0xff)) == 0) goto LAB_1000336c;
            bVar17 = uVar6 != 0;
            uVar6 = uVar6 - 1;
          } while (bVar17);
        }
      }
      else if (((int)uVar6 <= (int)uVar10) &&
              ((uVar8 & iVar9 << (uint)*(byte *)((int)param_1 + 5)) == 0)) {
LAB_1000336c:
        iVar11 = *param_1;
        if (*(char *)((int)param_1 + 6) == '\0') {
          uVar10 = iVar9 << (uVar6 & 0xff);
          uVar12 = uVar8 & uVar10;
          if ((uVar10 & uVar8) == 0) {
            if (uVar4 != 0) {
              do {
                uVar14 = (uint)*(ushort *)(iVar11 + uVar12 * 2);
                if (uVar14 < 0x2000) {
                  uVar14 = uVar14 + uVar6;
                }
                iVar13 = uVar12 + uVar6;
                uVar12 = uVar12 + 1;
                *(uint *)((iVar13 + 0x12) * 4 + iVar15) = uVar14;
              } while (uVar12 < uVar4);
            }
            *(uint *)(iVar5 + iVar7) = uVar8 | uVar10;
            *param_3 = iVar16;
            *param_4 = uVar6;
            FUN_1000174c(uVar3);
            return 1;
          }
        }
      }
      FUN_1000174c(uVar3);
      FUN_1000182c(uVar1,((uint)(*param_2 + DAT_10003438) >> 0x14) * 4 + iVar16);
    }
    iVar5 = 1;
    if (iVar13 == 1) {
      *param_2 = 0;
      return 0;
    }
    iVar13 = 1;
  } while( true );
}



/* ---------------------------------------------------------------- */
/* FUN_10003440 @ 10003440  (110 bytes) */
/* ---------------------------------------------------------------- */

undefined4 FUN_10003440(void)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  undefined4 uVar4;
  undefined4 in_r3;
  
  FUN_100036d4();
  iVar1 = DAT_100034b0;
  *(undefined4 *)(DAT_100034b0 + 0x78) = 0xc;
  FUN_10001958(5,DAT_100034b4,0xff,0xc,in_r3);
  iVar2 = DAT_100034b8;
  FUN_10002e10(DAT_100034b8,0,0x40);
  puVar3 = DAT_100034bc;
  *(undefined1 *)(iVar2 + 1) = 1;
  *(undefined2 *)(iVar2 + 0x18) = 0x40;
  *(undefined4 **)(iVar2 + 8) = puVar3;
  *puVar3 = 0;
  *(undefined1 *)(iVar2 + 0x21) = 0;
  *(undefined2 *)(iVar2 + 0x22) = 0x80;
  *(undefined1 *)(iVar2 + 0x3b) = 0;
  puVar3 = DAT_100034c4;
  uVar4 = DAT_100034c0;
  *(undefined4 *)(iVar2 + 4) = 0;
  *(undefined4 *)(iVar2 + 0xc) = uVar4;
  *(undefined2 *)(iVar2 + 0x38) = 0x40;
  *(undefined4 **)(iVar2 + 0x28) = puVar3;
  *puVar3 = 0;
  *(undefined4 *)(iVar2 + 0x24) = 0;
  *(undefined4 *)(iVar2 + 0x2c) = uVar4;
  FUN_10005228();
  *(undefined4 *)(iVar1 + 0x40) = 1;
  *(undefined4 *)(iVar1 + 0x4c) = 0x20000000;
  *(undefined4 *)(iVar1 + 0x90) = DAT_100034c8;
  *(undefined4 *)(DAT_100034cc + 0x4c) = 0x10000;
  return 1;
}



/* ---------------------------------------------------------------- */
/* FUN_100034d0 @ 100034d0  (12 bytes) */
/* ---------------------------------------------------------------- */

void FUN_100034d0(void)

{
  FUN_100018e0(5,1);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_100034dc @ 100034dc  (14 bytes) */
/* ---------------------------------------------------------------- */

void FUN_100034dc(void)

{
  FUN_10003714(DAT_100034ec,0,0);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_100034f0 @ 100034f0  (20 bytes) */
/* ---------------------------------------------------------------- */

void FUN_100034f0(undefined4 param_1,int param_2)

{
  *DAT_10003504 = (char)param_2;
  if (param_2 != 0) {
    *(undefined4 *)(DAT_10003508 + 0x90) = 0x20000;
  }
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_1000350c @ 1000350c  (22 bytes) */
/* ---------------------------------------------------------------- */

void FUN_1000350c(undefined4 param_1,byte *param_2)

{
  if (((*param_2 & 0x7f) == 0) && (param_2[1] == 5)) {
    *DAT_10003524 = (uint)param_2[2];
  }
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10003528 @ 10003528  (242 bytes) */
/* ---------------------------------------------------------------- */

undefined4 FUN_10003528(undefined4 param_1,int param_2)

{
  byte bVar1;
  ushort uVar2;
  uint uVar3;
  int iVar4;
  undefined4 *puVar5;
  uint uVar6;
  byte bVar7;
  uint uVar8;
  uint uVar9;
  int iVar10;
  int iVar11;
  
  iVar10 = DAT_1000361c;
  bVar7 = *(byte *)(param_2 + 3) & 3;
  if (bVar7 == 1) {
    return 0;
  }
  bVar1 = *(byte *)(param_2 + 2);
  uVar2 = *(ushort *)(param_2 + 4);
  uVar3 = bVar1 & 0x7f;
  uVar9 = (uint)(bVar1 >> 7);
  iVar11 = uVar3 * 2;
  iVar4 = DAT_1000361c + (iVar11 + uVar9) * 0x20;
  *(byte *)(iVar4 + 2) = bVar1;
  *(byte *)(iVar4 + 1) = bVar1 >> 7 ^ 1;
  uVar8 = uVar2 & 0x7ff;
  *(undefined1 *)(iVar4 + 3) = 0;
  *(short *)(iVar4 + 0x18) = (short)uVar8;
  *(byte *)(iVar4 + 0x1b) = bVar7;
  if (uVar9 == 1) {
    puVar5 = (undefined4 *)(uVar3 * 8 + DAT_10003634);
    iVar4 = uVar3 * 0x40 + iVar10;
    *(undefined4 **)(iVar4 + 0x28) = puVar5;
    *puVar5 = 0;
    if ((bVar1 & 0x7f) != 0) {
      *(uint *)(iVar4 + 0x24) = uVar3 * 8 + DAT_10003630;
      goto LAB_10003594;
    }
  }
  else {
    puVar5 = (undefined4 *)(uVar3 * 8 + DAT_10003620);
    iVar4 = uVar3 * 0x40 + iVar10;
    *(undefined4 **)(iVar4 + 8) = puVar5;
    *puVar5 = 0;
    if ((bVar1 & 0x7f) != 0) {
      *(uint *)(iVar4 + 4) = uVar3 * 8 + DAT_10003624;
LAB_10003594:
      uVar3 = uVar8 + 0x3f & 0xffffffc0;
      if (bVar7 == 2) {
        uVar3 = uVar3 << 1;
      }
      uVar6 = *DAT_10003628;
      *DAT_10003628 = uVar6 + uVar3;
      uVar8 = DAT_1000362c;
      iVar4 = iVar10 + (iVar11 + uVar9) * 0x20;
      *(uint *)(iVar4 + 0xc) = uVar6;
      if (uVar8 < uVar6 + uVar3) {
        FUN_10002928();
        uVar6 = *(uint *)(iVar4 + 0xc);
      }
      iVar10 = iVar10 + (iVar11 + uVar9) * 0x20;
      **(uint **)(iVar10 + 4) =
           (uint)*(byte *)(iVar10 + 0x1b) << 0x1a | uVar6 ^ DAT_10003630 | 0x80000000;
      return 1;
    }
  }
  iVar10 = iVar10 + uVar9 * 0x20;
  *(undefined4 *)(iVar10 + 4) = 0;
  *(undefined4 *)(iVar10 + 0xc) = DAT_10003638;
  return 1;
}



/* ---------------------------------------------------------------- */
/* FUN_1000363c @ 1000363c  (8 bytes) */
/* ---------------------------------------------------------------- */

void FUN_1000363c(void)

{
  FUN_10005228();
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10003644 @ 10003644  (32 bytes) */
/* ---------------------------------------------------------------- */

undefined4 FUN_10003644(undefined4 param_1,uint param_2,undefined4 param_3,undefined4 param_4)

{
  FUN_10003714(((param_2 & 0x7f) * 2 + (param_2 >> 7)) * 0x20 + DAT_10003664,param_3,param_4);
  return 1;
}



/* ---------------------------------------------------------------- */
/* FUN_10003668 @ 10003668  (50 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10003668(undefined4 param_1,uint param_2)

{
  if ((param_2 & 0x7f) == 0) {
    *(uint *)(DAT_1000369c + 0x68) = (param_2 == 0) + 1;
  }
  FUN_100052e8(((param_2 & 0x7f) * 2 + (param_2 >> 7)) * 0x20 + DAT_100036a0,0,0x800);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_100036a4 @ 100036a4  (40 bytes) */
/* ---------------------------------------------------------------- */

void FUN_100036a4(undefined4 param_1,uint param_2)

{
  undefined4 uVar1;
  int iVar2;
  
  uVar1 = DAT_100036d0;
  if ((param_2 & 0x7f) != 0) {
    iVar2 = DAT_100036cc + ((param_2 & 0x7f) * 2 + (param_2 >> 7)) * 0x20;
    *(undefined1 *)(iVar2 + 3) = 0;
    FUN_100052e8(iVar2,uVar1,0);
  }
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_100036d4 @ 100036d4  (44 bytes) */
/* ---------------------------------------------------------------- */

void FUN_100036d4(void)

{
  uint *puVar1;
  
  *DAT_10003700 = 0x1000000;
  puVar1 = DAT_10003708;
  *DAT_10003704 = 0x1000000;
  do {
  } while ((~*puVar1 & 0x1000000) != 0);
  FUN_10002e10(DAT_1000370c,0,0x1000);
  *(undefined4 *)(DAT_10003710 + 0x74) = 9;
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10003714 @ 10003714  (58 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10003714(int param_1,undefined4 param_2,undefined2 param_3)

{
  int iVar1;
  
  *(undefined2 *)(param_1 + 0x16) = 0;
  *(undefined2 *)(param_1 + 0x14) = param_3;
  *(undefined1 *)(param_1 + 0x1a) = 1;
  *(undefined4 *)(param_1 + 0x10) = param_2;
  iVar1 = FUN_10005258();
  if (iVar1 != 0) {
    *(undefined4 *)(DAT_10003750 + 0x90) = 0x20000;
  }
  iVar1 = FUN_10005268(param_1);
  if (iVar1 == 0) {
    FUN_100052d8(param_1);
  }
  else {
    *(undefined1 *)(param_1 + 0x1c) = 1;
  }
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10003754 @ 10003754  (4 bytes) */
/* ---------------------------------------------------------------- */

undefined4 FUN_10003754(void)

{
  return 0;
}



/* ---------------------------------------------------------------- */
/* FUN_1000375c @ 1000375c  (2 bytes) */
/* ---------------------------------------------------------------- */

void FUN_1000375c(void)

{
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10003760 @ 10003760  (4 bytes) */
/* ---------------------------------------------------------------- */

undefined4 FUN_10003760(void)

{
  return 0;
}



/* ---------------------------------------------------------------- */
/* FUN_10003764 @ 10003764  (4 bytes) */
/* ---------------------------------------------------------------- */

undefined4 FUN_10003764(void)

{
  return 0;
}



/* ---------------------------------------------------------------- */
/* FUN_10003768 @ 10003768  (4 bytes) */
/* ---------------------------------------------------------------- */

undefined4 FUN_10003768(void)

{
  return 0;
}



/* ---------------------------------------------------------------- */
/* FUN_1000376c @ 1000376c  (190 bytes) */
/* ---------------------------------------------------------------- */

undefined4 FUN_1000376c(undefined4 param_1,int param_2)

{
  char *pcVar1;
  byte *pbVar2;
  int *piVar3;
  undefined4 uVar4;
  uint uVar5;
  int iVar6;
  code *pcVar7;
  int iVar8;
  uint uVar9;
  
  pcVar1 = DAT_1000382c;
  if (*DAT_1000382c == -1) {
    if (param_2 != 0) {
      FUN_10002e10(DAT_10003830,0,0x54);
      uVar4 = DAT_10003834;
      *DAT_10003838 = 0;
      FUN_10001e34(uVar4);
      *DAT_1000383c = uVar4;
      iVar8 = DAT_10003840 + -0x14;
      FUN_10001e50(DAT_10003840);
      FUN_100050a0(iVar8);
      pbVar2 = DAT_10003848;
      *DAT_10003844 = iVar8;
      if (DAT_1000384c != 0) {
        *DAT_10003850 = (int)pbVar2;
      }
      iVar8 = DAT_10003854;
      piVar3 = DAT_10003850;
      uVar9 = 0;
      uVar5 = (uint)*pbVar2;
      do {
        if (uVar9 < uVar5) {
          iVar6 = *piVar3 + uVar9 * 0x20;
          if (iVar6 == 0) goto LAB_10003806;
          pcVar7 = *(code **)(iVar6 + 4);
        }
        else {
          pcVar7 = *(code **)((uVar9 - uVar5) * 0x20 + iVar8 + 4);
        }
        if (pcVar7 == (code *)0x0) goto LAB_10003806;
        (*pcVar7)();
        uVar5 = (uint)*pbVar2;
        uVar9 = uVar9 + 1 & 0xff;
      } while (uVar9 <= uVar5);
      *pcVar1 = (char)param_1;
      iVar8 = FUN_10003440(param_1,param_2);
      if (iVar8 != 0) {
        FUN_100034d0(param_1);
        goto LAB_10003820;
      }
    }
LAB_10003806:
    uVar4 = 0;
  }
  else {
LAB_10003820:
    uVar4 = 1;
  }
  return uVar4;
}



/* ---------------------------------------------------------------- */
/* FUN_10003858 @ 10003858  (122 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10003858(void)

{
  bool bVar1;
  int iVar2;
  int *piVar3;
  int iVar4;
  undefined4 uVar5;
  uint uVar6;
  int iVar7;
  undefined1 uStack_34;
  byte local_33;
  
  piVar3 = DAT_10003b98;
  iVar2 = DAT_10003b94;
  if (*DAT_10003b90 != -1) {
    while( true ) {
      iVar7 = *piVar3;
      uVar5 = 0;
      bVar1 = (bool)isCurrentModePrivileged();
      if (bVar1) {
        uVar5 = isIRQinterruptsEnabled();
      }
      disableIRQinterrupts();
      do {
      } while (**(int **)(iVar7 + 0x14) == 0);
      DataMemoryBarrier(0x1f);
      *(undefined4 *)(iVar7 + 0x18) = uVar5;
      iVar4 = FUN_10004f60(iVar7,&uStack_34);
      uVar6 = *(uint *)(iVar7 + 0x18);
      DataMemoryBarrier(0x1f);
      **(undefined4 **)(iVar7 + 0x14) = 0;
      bVar1 = (bool)isCurrentModePrivileged();
      if (bVar1) {
        enableIRQinterrupts((uVar6 & 1) == 1);
      }
      if (iVar4 == 0) break;
      if (local_33 < 9) {
                    /* WARNING: Could not recover jumptable at 0x100038c2. Too many branches */
                    /* WARNING: Treating indirect jump as call */
        (**(code **)(iVar2 + (uint)local_33 * 4))();
        return;
      }
    }
  }
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10004018 @ 10004018  (148 bytes) */
/* ---------------------------------------------------------------- */

undefined4
FUN_10004018(undefined4 param_1,byte *param_2,int param_3,uint param_4,byte *param_5,byte *param_6)

{
  undefined1 uVar1;
  byte bVar2;
  undefined1 *puVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  
  iVar4 = DAT_100040b0;
  puVar3 = DAT_100040ac;
  if (param_3 != 0) {
    iVar6 = 0;
    do {
      while( true ) {
        if (((param_2[1] != 5) || ((param_2[3] & 3) != param_4)) || ((param_2[2] & 0x70) != 0)) {
          return 0;
        }
        uVar1 = *puVar3;
        iVar5 = FUN_10004eb0(param_2,*(undefined1 *)(iVar4 + 2));
        if (iVar5 == 0) {
          return 0;
        }
        iVar5 = FUN_10003528(uVar1,param_2);
        if (iVar5 == 0) {
          return 0;
        }
        bVar2 = param_2[2];
        if (-1 < (char)bVar2) break;
        iVar6 = iVar6 + 1;
        *param_6 = bVar2;
        param_2 = param_2 + *param_2;
        if (param_3 <= iVar6) {
          return 1;
        }
      }
      iVar6 = iVar6 + 1;
      *param_5 = bVar2;
      param_2 = param_2 + *param_2;
    } while (iVar6 < param_3);
  }
  return 1;
}



/* ---------------------------------------------------------------- */
/* FUN_100040b4 @ 100040b4  (66 bytes) */
/* ---------------------------------------------------------------- */

int FUN_100040b4(undefined4 param_1,uint param_2)

{
  undefined1 uVar1;
  int iVar2;
  int iVar3;
  
  iVar3 = DAT_100040f8 + (param_2 & 0x7f) * 2 + (param_2 >> 7);
  if (-1 < (int)((uint)*(byte *)(iVar3 + 0x34) << 0x1f)) {
    uVar1 = *DAT_100040fc;
    *(byte *)(iVar3 + 0x34) = *(byte *)(iVar3 + 0x34) | 1;
    iVar2 = FUN_10003644(uVar1);
    if (iVar2 != 0) {
      return iVar2;
    }
    *(byte *)(iVar3 + 0x34) = *(byte *)(iVar3 + 0x34) & 0xfe;
    *(byte *)(iVar3 + 0x34) = *(byte *)(iVar3 + 0x34) & 0xfb;
  }
  return 0;
}



/* ---------------------------------------------------------------- */
/* FUN_10004100 @ 10004100  (24 bytes) */
/* ---------------------------------------------------------------- */

byte FUN_10004100(undefined4 param_1,uint param_2)

{
  return *(byte *)(DAT_10004118 + (param_2 & 0x7f) * 2 + (param_2 >> 7) + 0x34) & 1;
}



/* ---------------------------------------------------------------- */
/* FUN_1000411c @ 1000411c  (46 bytes) */
/* ---------------------------------------------------------------- */

void FUN_1000411c(undefined4 param_1,uint param_2)

{
  int iVar1;
  
  FUN_10003668(*DAT_1000414c);
  iVar1 = DAT_10004150 + (param_2 & 0x7f) * 2 + (param_2 >> 7);
  *(byte *)(iVar1 + 0x34) = *(byte *)(iVar1 + 0x34) | 2;
  *(byte *)(iVar1 + 0x34) = *(byte *)(iVar1 + 0x34) | 1;
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10004154 @ 10004154  (24 bytes) */
/* ---------------------------------------------------------------- */

uint FUN_10004154(undefined4 param_1,uint param_2)

{
  return (*(byte *)(DAT_1000416c + (param_2 & 0x7f) * 2 + (param_2 >> 7) + 0x34) & 3) >> 1;
}



/* ---------------------------------------------------------------- */
/* FUN_10004170 @ 10004170  (40 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10004170(undefined4 param_1,byte *param_2)

{
  int iVar1;
  
  iVar1 = DAT_10004198;
  FUN_10002e1c(DAT_10004198,param_2,8);
  *(undefined4 *)(iVar1 + 0xc) = 0;
  *(undefined4 *)(iVar1 + 8) = 0;
  FUN_100040b4(param_1,~*param_2 & 0x80,0,0);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_1000419c @ 1000419c  (160 bytes) */
/* ---------------------------------------------------------------- */

undefined4 FUN_1000419c(undefined4 param_1,byte *param_2,int param_3,uint param_4)

{
  byte *pbVar1;
  undefined4 uVar2;
  byte bVar3;
  ushort uVar4;
  uint uVar5;
  
  pbVar1 = DAT_1000423c;
  FUN_10002e1c(DAT_1000423c,param_2,8);
  *(int *)(pbVar1 + 8) = param_3;
  pbVar1[0xe] = 0;
  pbVar1[0xf] = 0;
  uVar4 = *(ushort *)(param_2 + 6);
  uVar5 = (uint)uVar4;
  if (param_4 < uVar5) {
    uVar4 = (ushort)param_4;
  }
  *(ushort *)(pbVar1 + 0xc) = uVar4;
  uVar2 = DAT_10004240;
  if (uVar5 == 0) {
    uVar2 = FUN_100040b4(param_1,~*param_2 & 0x80,0,0);
  }
  else {
    if (uVar4 == 0) {
      uVar2 = 0;
      bVar3 = *pbVar1 & 0x80;
    }
    else {
      if (param_3 == 0) {
        return 0;
      }
      if (uVar4 < 0x41) {
        bVar3 = *pbVar1;
      }
      else {
        uVar4 = 0x40;
        bVar3 = *pbVar1;
      }
      if (bVar3 < 0x80) {
        bVar3 = 0;
      }
      else {
        FUN_10002e1c(DAT_10004240,param_3,uVar4);
        bVar3 = 0x80;
      }
    }
    uVar2 = FUN_100040b4(param_1,bVar3,uVar2,uVar4);
  }
  return uVar2;
}



/* ---------------------------------------------------------------- */
/* FUN_10004244 @ 10004244  (14 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10004244(void)

{
  FUN_10002e10(DAT_10004254,0,0x14);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10004258 @ 10004258  (6 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10004258(undefined4 param_1)

{
  *(undefined4 *)(DAT_10004260 + 0x10) = param_1;
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10004264 @ 10004264  (22 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10004264(undefined4 param_1)

{
  int iVar1;
  
  iVar1 = DAT_1000427c;
  FUN_10002e1c(DAT_1000427c,param_1,8);
  *(undefined4 *)(iVar1 + 8) = 0;
  *(undefined4 *)(iVar1 + 0xc) = 0;
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10004280 @ 10004280  (226 bytes) */
/* ---------------------------------------------------------------- */

undefined4 FUN_10004280(undefined4 param_1,uint param_2,undefined4 param_3,uint param_4)

{
  byte *pbVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 uVar4;
  uint uVar5;
  uint uVar6;
  undefined2 uVar7;
  
  pbVar1 = DAT_10004364;
  if ((uint)(*DAT_10004364 >> 7) == param_2 >> 7) {
    if (*DAT_10004364 < 0x80) {
      if (*(int *)(DAT_10004364 + 8) == 0) {
        return 0;
      }
      FUN_10002e1c(*(int *)(DAT_10004364 + 8),DAT_10004368,param_4);
    }
    iVar3 = *(int *)(pbVar1 + 8);
    uVar5 = *(ushort *)(pbVar1 + 0xe) + param_4 & 0xffff;
    *(short *)(pbVar1 + 0xe) = (short)(*(ushort *)(pbVar1 + 0xe) + param_4);
    *(uint *)(pbVar1 + 8) = iVar3 + param_4;
    uVar2 = DAT_10004368;
    if ((*(ushort *)(pbVar1 + 6) == uVar5) || (param_4 < 0x40)) {
      if ((*(code **)(pbVar1 + 0x10) == (code *)0x0) ||
         (iVar3 = (**(code **)(pbVar1 + 0x10))(param_1,2,pbVar1), iVar3 != 0)) {
        uVar2 = FUN_100040b4(param_1,~*pbVar1 & 0x80,0,0);
      }
      else {
        FUN_10003668(param_1,0);
        FUN_10003668(param_1,0x80);
        uVar2 = 1;
      }
    }
    else {
      uVar6 = (uint)*(ushort *)(pbVar1 + 0xc);
      uVar7 = (undefined2)(uVar6 - uVar5);
      if (0x40 < (uVar6 - uVar5 & 0xffff)) {
        uVar7 = 0x40;
      }
      if (*pbVar1 < 0x80) {
        uVar4 = 0;
        if (uVar5 == uVar6) {
          uVar2 = 0;
        }
      }
      else if (uVar5 == uVar6) {
        uVar4 = 0x80;
        uVar2 = 0;
      }
      else {
        FUN_10002e1c(DAT_10004368,iVar3 + param_4,uVar7);
        uVar4 = 0x80;
      }
      uVar2 = FUN_100040b4(param_1,uVar4,uVar2,uVar7);
    }
  }
  else {
    uVar2 = 0;
    if (param_4 == 0) {
      FUN_1000350c(param_1,DAT_10004364);
      if (*(code **)(pbVar1 + 0x10) != (code *)0x0) {
        (**(code **)(pbVar1 + 0x10))(param_1,3,pbVar1);
      }
      uVar2 = 1;
    }
  }
  return uVar2;
}



/* ---------------------------------------------------------------- */
/* FUN_1000436c @ 1000436c  (72 bytes) */
/* ---------------------------------------------------------------- */

void FUN_1000436c(undefined4 param_1,undefined1 param_2)

{
  int iVar1;
  int iVar2;
  
  iVar1 = DAT_100043b4;
  *(undefined1 *)(DAT_100043b4 + 0x2c) = param_2;
  *(undefined1 *)(iVar1 + 0x30) = 2;
  iVar2 = *(int *)(iVar1 + 8) - *(int *)(iVar1 + 0x38);
  *(int *)(iVar1 + 0x28) = iVar2;
  if (*(char *)(iVar1 + 0x3c) == '\0') {
    *(short *)(iVar1 + 0x3c) = (short)DAT_100043b8;
    *(undefined1 *)(iVar1 + 0x3e) = 0;
  }
  if ((*(int *)(iVar1 + 8) != 0) && (iVar2 != 0)) {
    if (*(char *)(iVar1 + 0xc) < '\0') {
      FUN_1000411c(param_1,*(undefined1 *)(iVar1 + 0x2e));
    }
    else {
      FUN_1000411c(param_1,*(undefined1 *)(iVar1 + 0x2f));
    }
  }
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_100043bc @ 100043bc  (218 bytes) */
/* ---------------------------------------------------------------- */

void FUN_100043bc(undefined4 param_1)

{
  byte bVar1;
  ushort uVar2;
  int iVar3;
  undefined4 uVar4;
  undefined2 uVar5;
  int iVar6;
  uint uVar7;
  uint uVar8;
  int iVar9;
  uint uVar10;
  undefined8 uVar11;
  undefined1 local_2c;
  undefined1 local_2b;
  undefined1 local_28;
  undefined1 local_27;
  uint local_24;
  
  iVar3 = DAT_10004498;
  iVar6 = *(int *)(DAT_10004498 + 8);
  uVar2 = *(ushort *)(DAT_10004498 + 0x16);
  if (uVar2 == 0) {
    uVar5 = 0;
  }
  else {
    uVar5 = FUN_10002b7c(iVar6,uVar2 << 8 | uVar2 >> 8);
  }
  uVar7 = *(uint *)(iVar3 + 0x10);
  bVar1 = *(byte *)(iVar3 + 0x14);
  uVar8 = uVar7 >> 8;
  iVar9 = *(int *)(iVar3 + 0x38);
  uVar11 = FUN_10002b7c(iVar9,uVar5);
  uVar4 = DAT_1000449c;
  uVar10 = iVar6 - iVar9;
  if (0x200 < uVar10) {
    uVar10 = 0x200;
  }
  local_24 = FUN_10000354(*(undefined1 *)(iVar3 + 0xd),
                          (int)uVar11 +
                          (uVar8 << 0x18 | ((uVar8 & 0xff00) >> 8) << 0x10 | (uVar7 >> 0x18) << 8 |
                          (uint)bVar1),(int)((ulonglong)uVar11 >> 0x20),DAT_1000449c,uVar10);
  if ((int)local_24 < 0) {
    *(short *)(iVar3 + 0x3c) = (short)DAT_100044a0;
    *(undefined1 *)(iVar3 + 0x3e) = 0;
    *(undefined1 *)(iVar3 + 0x2c) = 1;
    *(undefined1 *)(iVar3 + 0x30) = 2;
    iVar6 = *(int *)(iVar3 + 8) - *(int *)(iVar3 + 0x38);
    *(int *)(iVar3 + 0x28) = iVar6;
    if ((*(int *)(iVar3 + 8) != 0) && (iVar6 != 0)) {
      if (*(char *)(iVar3 + 0xc) < '\0') {
        FUN_1000411c(param_1,*(undefined1 *)(iVar3 + 0x2e));
      }
      else {
        FUN_1000411c(param_1,*(undefined1 *)(iVar3 + 0x2f));
      }
    }
  }
  else {
    local_28 = *(undefined1 *)(iVar3 + 0x2e);
    if (local_24 == 0) {
      local_2c = (undefined1)param_1;
      local_2b = 7;
      local_27 = 0;
      FUN_100052b8(&local_2c,0);
    }
    else {
      FUN_100040b4(param_1,local_28,uVar4,local_24 & 0xffff);
    }
  }
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10004534 @ 10004534  (312 bytes) */
/* ---------------------------------------------------------------- */

undefined4 FUN_10004534(int param_1,int param_2,byte *param_3)

{
  char cVar1;
  byte bVar2;
  int iVar3;
  undefined4 *puVar4;
  undefined4 uVar5;
  int iVar6;
  undefined4 uVar7;
  char local_19 [5];
  
  iVar3 = DAT_10004670;
  if (param_2 == 1) {
    if (((*param_3 & 0x7f) == 2) && (param_3[1] == 1)) {
      if (*(short *)(param_3 + 2) == 0) {
        cVar1 = *(char *)(DAT_10004670 + 0x30);
        if (cVar1 == '\x04') {
          FUN_1000411c();
          return 1;
        }
        bVar2 = *(byte *)(DAT_10004670 + 0x2e);
        if (bVar2 == param_3[4]) {
          if (cVar1 == '\x02') {
            *(int *)(DAT_10004670 + 0x28) =
                 *(int *)(DAT_10004670 + 8) - *(int *)(DAT_10004670 + 0x38);
            *(undefined1 *)(iVar3 + 0x30) = 3;
            puVar4 = DAT_10004674;
            uVar5 = *(undefined4 *)(iVar3 + 0x24);
            uVar7 = *(undefined4 *)(iVar3 + 0x28);
            *DAT_10004674 = *(undefined4 *)(iVar3 + 0x20);
            puVar4[1] = uVar5;
            puVar4[2] = uVar7;
            *(undefined1 *)(puVar4 + 3) = *(undefined1 *)(iVar3 + 0x2c);
            uVar5 = FUN_100040b4(param_1,bVar2,puVar4,0xd);
            return uVar5;
          }
        }
        else {
          bVar2 = *(byte *)(DAT_10004670 + 0x2f);
          if (((bVar2 == param_3[4]) && (cVar1 == '\0')) &&
             ((iVar6 = FUN_10004100(param_1,bVar2), iVar6 == 0 &&
              (iVar6 = FUN_10004154(param_1,bVar2), iVar6 == 0)))) {
            *(undefined1 *)(iVar3 + 0x30) = 0;
            uVar5 = FUN_100040b4(param_1,*(undefined1 *)(iVar3 + 0x2f),DAT_10004674,0x1f);
            return uVar5;
          }
        }
        goto LAB_10004540;
      }
    }
    else if ((*param_3 & 0x60) == 0x20) {
      if (param_3[1] == 0xfe) {
        if ((*(short *)(param_3 + 2) == 0) && (*(short *)(param_3 + 6) == 1)) {
          if (DAT_10004678 == 0) {
            local_19[0] = '\0';
LAB_10004606:
            FUN_1000419c(param_1,param_3,local_19,1);
            return 1;
          }
          if (param_1 != 0) {
            local_19[0] = (char)param_1 + -1;
            goto LAB_10004606;
          }
        }
      }
      else if (((param_3[1] == 0xff) && (*(short *)(param_3 + 2) == 0)) &&
              (*(short *)(param_3 + 6) == 0)) {
        *(undefined1 *)(DAT_10004670 + 0x30) = 0;
        *(undefined4 *)(iVar3 + 0x34) = 0;
        *(undefined4 *)(iVar3 + 0x38) = 0;
        *(undefined2 *)(iVar3 + 0x3c) = 0;
        *(undefined1 *)(iVar3 + 0x3e) = 0;
        FUN_10004170(param_1,param_3);
        return 1;
      }
    }
    uVar5 = 0;
  }
  else {
LAB_10004540:
    uVar5 = 1;
  }
  return uVar5;
}



/* ---------------------------------------------------------------- */
/* FUN_1000467c @ 1000467c  (1478 bytes) */
/* ---------------------------------------------------------------- */

undefined4 FUN_1000467c(undefined4 param_1,uint param_2,undefined4 param_3,uint param_4)

{
  uint *puVar1;
  byte bVar2;
  undefined1 uVar3;
  ushort uVar4;
  int *piVar5;
  undefined2 uVar6;
  undefined4 uVar7;
  uint uVar8;
  uint uVar9;
  int iVar10;
  char cVar11;
  int iVar12;
  int *piVar13;
  int *piVar14;
  undefined8 uVar15;
  undefined1 local_4c;
  undefined1 local_4b;
  undefined1 local_48;
  undefined1 local_47;
  int local_44;
  
  piVar13 = DAT_10004964;
  piVar5 = DAT_10004960;
  cVar11 = (char)DAT_10004960[0xc];
  if (cVar11 == '\x01') {
    iVar10 = DAT_10004960[0xe];
    if (*(char *)((int)DAT_10004960 + 0xf) == '(') {
      puVar1 = (uint *)(DAT_10004960 + 0xd);
      DAT_10004960[0xe] = param_4 + iVar10;
      if (*puVar1 <= param_4 + iVar10) goto LAB_100047c0;
      FUN_100043bc();
      cVar11 = (char)piVar5[0xc];
      goto LAB_10004742;
    }
    if (*(char *)((int)DAT_10004960 + 0xf) != '*') {
      piVar13 = DAT_10004960 + 3;
      puVar1 = (uint *)(DAT_10004960 + 0xd);
      DAT_10004960[0xe] = param_4 + iVar10;
      if ((char)*piVar13 < '\0') {
        if (param_4 + iVar10 < *puVar1) {
          return 1;
        }
      }
      else {
        iVar10 = FUN_10000894(*(undefined1 *)((int)piVar5 + 0xd),(int)piVar5 + 0xf,DAT_10004964,
                              *puVar1 & 0xffff);
        if (iVar10 < 0) {
          *(undefined1 *)(piVar5 + 0xb) = 1;
          *(undefined1 *)(piVar5 + 0xc) = 2;
          iVar10 = piVar5[2] - piVar5[0xe];
          piVar5[10] = iVar10;
          if ((char)piVar5[0xf] == '\0') {
            *(short *)(piVar5 + 0xf) = (short)DAT_10004cec;
            *(undefined1 *)((int)piVar5 + 0x3e) = 0;
          }
          if ((piVar5[2] == 0) || (iVar10 == 0)) {
            if ((uint)piVar5[0xe] < (uint)piVar5[0xd]) goto LAB_100047c6;
            goto LAB_100047c0;
          }
          if ((char)piVar5[3] < '\0') {
            FUN_1000411c(param_1,*(undefined1 *)((int)piVar5 + 0x2e));
          }
          else {
            FUN_1000411c(param_1,*(undefined1 *)((int)piVar5 + 0x2f));
          }
        }
        if ((uint)piVar5[0xe] < (uint)piVar5[0xd]) goto LAB_1000473e;
      }
LAB_100047c0:
      *(undefined1 *)(piVar5 + 0xc) = 2;
      goto LAB_100047c6;
    }
    uVar4 = *(ushort *)((int)DAT_10004960 + 0x16);
    if (uVar4 == 0) {
      uVar6 = 0;
    }
    else {
      uVar6 = FUN_10002b7c(DAT_10004960[2],uVar4 << 8 | uVar4 >> 8);
    }
    uVar9 = piVar5[4];
    bVar2 = *(byte *)(piVar5 + 5);
    uVar8 = uVar9 >> 8;
    uVar15 = FUN_10002b7c(iVar10,uVar6);
    piVar13 = DAT_10004964;
    uVar9 = FUN_1000053c(*(undefined1 *)((int)piVar5 + 0xd),
                         (int)uVar15 +
                         (uVar8 << 0x18 | ((uVar8 & 0xff00) >> 8) << 0x10 | (uVar9 >> 0x18) << 8 |
                         (uint)bVar2),(int)((ulonglong)uVar15 >> 0x20),DAT_10004964,param_4);
    if ((int)uVar9 < 0) {
      *(short *)(piVar5 + 0xf) = (short)DAT_10004cf4;
      *(undefined1 *)((int)piVar5 + 0x3e) = 0;
      iVar10 = piVar5[0xe];
      *(undefined1 *)(piVar5 + 0xb) = 1;
      piVar5[0xe] = param_4 + iVar10;
      iVar10 = piVar5[2] - (param_4 + iVar10);
      piVar5[10] = iVar10;
      *(undefined1 *)(piVar5 + 0xc) = 2;
      if ((piVar5[2] == 0) || (iVar10 == 0)) goto LAB_100047c6;
      if ((char)piVar5[3] < '\0') {
        FUN_1000411c(param_1,*(undefined1 *)((int)piVar5 + 0x2e));
        cVar11 = (char)piVar5[0xc];
      }
      else {
        FUN_1000411c(param_1,*(undefined1 *)((int)piVar5 + 0x2f));
        cVar11 = (char)piVar5[0xc];
      }
    }
    else {
      if (param_4 <= uVar9) {
        param_4 = param_4 + piVar5[0xe];
        piVar5[0xe] = param_4;
        if ((uint)piVar5[0xd] <= param_4) goto LAB_100047c0;
        if (PTR_FUN_10000350_1_10004978 != (undefined *)0x0) {
          iVar10 = FUN_10000350(*(undefined1 *)((int)piVar5 + 0xd));
          if (iVar10 == 0) {
LAB_10004dde:
            *(short *)(piVar5 + 0xf) = (short)DAT_10004e5c;
            *(undefined1 *)((int)piVar5 + 0x3e) = 0;
            FUN_1000436c(param_1,1);
            cVar11 = (char)piVar5[0xc];
            goto LAB_10004742;
          }
          param_4 = piVar5[0xe];
        }
        uVar9 = piVar5[2] - param_4;
        if (0x200 < piVar5[2] - param_4) {
          uVar9 = 0x200;
        }
        param_2 = (uint)*(byte *)((int)piVar5 + 0x2f);
LAB_10004948:
        FUN_100040b4(param_1,param_2,piVar13,uVar9 & 0xffff);
        goto LAB_1000473e;
      }
      if (uVar9 != 0) {
        piVar5[0xe] = piVar5[0xe] + (uVar9 & 0xffff);
        FUN_10005104(piVar13,uVar9 + (int)piVar13,param_4 - uVar9);
      }
      local_48 = *(undefined1 *)((int)piVar5 + 0x2f);
      local_4c = (undefined1)param_1;
      local_47 = 0;
      local_4b = 7;
      local_44 = param_4 - uVar9;
      FUN_100052b8(&local_4c,0);
      cVar11 = (char)piVar5[0xc];
    }
  }
  else if (cVar11 == '\x03') {
    if (*(byte *)((int)DAT_10004960 + 0x2e) != param_2) {
      return 1;
    }
    if (param_4 != 0xd) {
      return 1;
    }
    if (((*(char *)((int)DAT_10004960 + 0xf) != '(') && (*(char *)((int)DAT_10004960 + 0xf) == '*'))
       && (DAT_10004d0c != 0)) {
      FUN_10000890(*(undefined1 *)((int)DAT_10004960 + 0xd));
    }
    *(undefined1 *)(piVar5 + 0xc) = 0;
    iVar10 = FUN_100040b4(param_1,*(undefined1 *)((int)piVar5 + 0x2f),DAT_10004964,0x1f);
    if (iVar10 == 0) {
      return 0;
    }
    cVar11 = (char)piVar5[0xc];
  }
  else if (cVar11 == '\0') {
    if (*(byte *)((int)DAT_10004960 + 0x2f) != param_2) {
      return 1;
    }
    if ((param_4 != 0x1f) || (*DAT_10004964 != DAT_10004968)) {
      *(undefined1 *)(DAT_10004960 + 0xc) = 4;
      FUN_1000411c(param_1,*(undefined1 *)((int)piVar5 + 0x2e));
      FUN_1000411c(param_1,*(undefined1 *)((int)piVar5 + 0x2f));
      return 0;
    }
    iVar10 = DAT_10004964[1];
    iVar12 = DAT_10004964[2];
    piVar14 = DAT_10004964 + 3;
    *DAT_10004960 = *DAT_10004964;
    piVar5[1] = iVar10;
    piVar5[2] = iVar12;
    iVar10 = piVar13[4];
    iVar12 = piVar13[5];
    piVar5[3] = *piVar14;
    piVar5[4] = iVar10;
    piVar5[5] = iVar12;
    piVar5[6] = piVar13[6];
    *(short *)(piVar5 + 7) = (short)piVar13[7];
    *(undefined1 *)((int)piVar5 + 0x1e) = *(undefined1 *)((int)piVar13 + 0x1e);
    piVar5[8] = DAT_1000496c;
    *(undefined1 *)(piVar5 + 0xb) = 0;
    piVar5[9] = piVar5[1];
    *(undefined1 *)(piVar5 + 0xc) = 1;
    uVar8 = (uint)*(byte *)((int)piVar5 + 0xf);
    uVar9 = piVar5[2];
    piVar5[10] = 0;
    piVar5[0xd] = uVar9;
    piVar5[0xe] = 0;
    if (uVar8 == 0x28) {
      uVar8 = (uint)*(ushort *)((int)piVar5 + 0x16);
      if (uVar9 == 0) {
LAB_10004be0:
        if (uVar8 != 0) {
          *(undefined1 *)(piVar5 + 0xb) = 2;
          piVar5[10] = 0;
          *(undefined1 *)(piVar5 + 0xc) = 2;
          cVar11 = (char)piVar5[0xf];
joined_r0x10004bfa:
          if (cVar11 == '\0') {
            *(short *)(piVar5 + 0xf) = (short)DAT_10004cec;
            *(undefined1 *)((int)piVar5 + 0x3e) = 0;
          }
          goto LAB_100047c6;
        }
        goto LAB_100047c0;
      }
      if ((char)piVar5[3] < '\0') {
        if (uVar8 == 0) {
          piVar5[10] = uVar9;
          *(undefined1 *)(piVar5 + 0xb) = 1;
          *(undefined1 *)(piVar5 + 0xc) = 2;
          if ((char)piVar5[0xf] != '\0') goto LAB_1000490e;
        }
        else {
          if (((uVar8 & 0xff) << 8 | (uint)(*(ushort *)((int)piVar5 + 0x16) >> 8)) <= uVar9) {
            FUN_100043bc(param_1);
            cVar11 = (char)piVar5[0xc];
            goto LAB_10004742;
          }
LAB_10004d34:
          piVar5[10] = uVar9;
          *(undefined1 *)(piVar5 + 0xb) = 2;
          *(undefined1 *)(piVar5 + 0xc) = 2;
          if ((char)piVar5[0xf] != '\0') goto LAB_10004904;
        }
LAB_100048fc:
        *(short *)(piVar5 + 0xf) = (short)DAT_10004970;
        *(undefined1 *)((int)piVar5 + 0x3e) = 0;
      }
      else {
        piVar5[10] = uVar9;
        *(undefined1 *)(piVar5 + 0xc) = 2;
        *(undefined1 *)(piVar5 + 0xb) = 2;
        if ((char)piVar5[0xf] != '\0') goto LAB_10004738;
        *(short *)(piVar5 + 0xf) = (short)DAT_10004e58;
        *(undefined1 *)((int)piVar5 + 0x3e) = 0;
      }
LAB_10004904:
      if ((char)piVar5[3] < '\0') {
LAB_1000490e:
        param_2 = (uint)*(byte *)((int)piVar5 + 0x2e);
      }
    }
    else {
      if (uVar8 == 0x2a) {
        uVar4 = *(ushort *)((int)piVar5 + 0x16);
        uVar8 = (uint)uVar4;
        if (uVar9 != 0) {
          if ((char)piVar5[3] < '\0') {
            piVar5[10] = uVar9;
            *(undefined1 *)(piVar5 + 0xb) = 2;
            *(undefined1 *)(piVar5 + 0xc) = 2;
            if ((char)piVar5[0xf] == '\0') goto LAB_100048fc;
            goto LAB_1000490e;
          }
          if (uVar8 == 0) {
            piVar5[10] = uVar9;
            *(undefined1 *)(piVar5 + 0xb) = 1;
            *(undefined1 *)(piVar5 + 0xc) = 2;
            if ((char)piVar5[0xf] == '\0') {
              *(short *)(piVar5 + 0xf) = (short)DAT_10004cec;
              *(char *)((int)piVar5 + 0x3e) = (char)uVar4;
              goto LAB_10004904;
            }
            goto LAB_10004738;
          }
          if (uVar9 < ((uVar8 & 0xff) << 8 | (uint)(uVar4 >> 8))) goto LAB_10004d34;
          if (DAT_10004e60 != 0) {
            iVar10 = FUN_10000350(*(undefined1 *)((int)piVar5 + 0xd));
            if (iVar10 == 0) goto LAB_10004dde;
            uVar9 = piVar5[2] - piVar5[0xe];
            param_2 = (uint)*(byte *)((int)piVar5 + 0x2f);
          }
          if (0x200 < uVar9) {
            uVar9 = 0x200;
          }
          goto LAB_10004948;
        }
        goto LAB_10004be0;
      }
      if ((uVar9 != 0) && (-1 < (char)piVar5[3])) {
        if (0x200 < uVar9) {
          piVar5[10] = uVar9;
          *(undefined1 *)(piVar5 + 0xb) = 1;
          *(undefined1 *)(piVar5 + 0xc) = 2;
          if ((char)piVar5[0xf] == '\0') {
            *(short *)(piVar5 + 0xf) = (short)DAT_10004970;
            *(undefined1 *)((int)piVar5 + 0x3e) = 0;
          }
          goto LAB_10004738;
        }
        iVar10 = FUN_100040b4(param_1,param_2,piVar13,uVar9 & 0xffff);
        if (iVar10 == 0) {
          return 0;
        }
        goto LAB_1000473e;
      }
      if (uVar8 < 0x26) {
                    /* WARNING: Could not recover jumptable at 0x1000495e. Too many branches */
                    /* WARNING: Treating indirect jump as call */
        uVar7 = (**(code **)(PTR_LAB_1000497c + uVar8 * 4))();
        return uVar7;
      }
      if ((char)piVar5[0xf] == '\0') {
        uVar9 = FUN_10000894(*(undefined1 *)((int)piVar5 + 0xd),DAT_10004ce8,piVar13,uVar9 & 0xffff)
        ;
        if ((int)uVar9 < 0) {
          uVar8 = piVar5[2];
          uVar9 = uVar8 - piVar5[0xe];
          *(undefined1 *)(piVar5 + 0xb) = 1;
          piVar5[10] = uVar9;
          *(undefined1 *)(piVar5 + 0xc) = 2;
          if ((char)piVar5[0xf] == '\0') {
            *(short *)(piVar5 + 0xf) = (short)DAT_10004cec;
            *(undefined1 *)((int)piVar5 + 0x3e) = 0;
          }
          goto LAB_10004aca;
        }
        if (uVar9 != 0) {
          uVar8 = piVar5[2];
          if (uVar8 != 0) {
            if (uVar8 < uVar9) {
              uVar9 = uVar8;
            }
            piVar5[0xd] = uVar9;
            iVar10 = FUN_100040b4(param_1,*(undefined1 *)((int)piVar5 + 0x2e),piVar13,uVar9 & 0xffff
                                 );
            if (iVar10 == 0) {
              return 0;
            }
            goto LAB_1000473e;
          }
          *(undefined1 *)(piVar5 + 0xb) = 1;
          piVar5[10] = -piVar5[0xe];
          *(undefined1 *)(piVar5 + 0xc) = 2;
          cVar11 = (char)piVar5[0xf];
          goto joined_r0x10004bfa;
        }
        if (piVar5[2] == 0) goto LAB_100047c0;
        *(undefined1 *)(piVar5 + 0xb) = 1;
        uVar9 = piVar5[2] - piVar5[0xe];
        piVar5[10] = uVar9;
        *(undefined1 *)(piVar5 + 0xc) = 2;
        if ((char)piVar5[0xf] == '\0') {
          *(short *)(piVar5 + 0xf) = (short)DAT_10004cec;
          *(undefined1 *)((int)piVar5 + 0x3e) = 0;
        }
      }
      else {
        *(undefined1 *)(piVar5 + 0xb) = 1;
        *(undefined1 *)(piVar5 + 0xc) = 2;
        piVar5[10] = uVar9;
        uVar8 = uVar9;
LAB_10004aca:
        if (uVar8 == 0) goto LAB_100047c6;
      }
      if (uVar9 == 0) goto LAB_100047c6;
      if ((char)piVar5[3] < '\0') goto LAB_1000490e;
      param_2 = (uint)*(byte *)((int)piVar5 + 0x2f);
    }
LAB_10004738:
    FUN_1000411c(param_1,param_2);
LAB_1000473e:
    cVar11 = (char)piVar5[0xc];
  }
LAB_10004742:
  if (cVar11 != '\x02') {
    return 1;
  }
LAB_100047c6:
  iVar10 = FUN_10004154(param_1,*(undefined1 *)((int)piVar5 + 0x2e));
  if (iVar10 != 0) {
    return 1;
  }
  uVar3 = *(undefined1 *)((int)piVar5 + 0x2e);
  if (((uint)piVar5[0xe] < (uint)piVar5[2]) && ((char)piVar5[3] < '\0')) {
    FUN_1000411c(param_1,uVar3);
    return 1;
  }
  piVar5[10] = piVar5[2] - piVar5[0xe];
  *(undefined1 *)(piVar5 + 0xc) = 3;
  piVar13 = DAT_10004964;
  iVar10 = piVar5[9];
  iVar12 = piVar5[10];
  *DAT_10004964 = piVar5[8];
  piVar13[1] = iVar10;
  piVar13[2] = iVar12;
  *(char *)(piVar13 + 3) = (char)piVar5[0xb];
  uVar7 = FUN_100040b4(param_1,uVar3,piVar13,0xd);
  return uVar7;
}



/* ---------------------------------------------------------------- */
/* FUN_10004e64 @ 10004e64  (70 bytes) */
/* ---------------------------------------------------------------- */

int FUN_10004e64(uint param_1,char *param_2)

{
  char cVar1;
  int iVar2;
  undefined2 local_c [4];
  
  if (param_2 == (char *)0x0) {
    local_c[0] = 1;
    iVar2 = FUN_1000376c(0,local_c);
    if (iVar2 != 0) {
      *DAT_10004eac = 1;
    }
  }
  else {
    iVar2 = 0;
    if (param_1 < 2) {
      cVar1 = *param_2;
      if (cVar1 != '\0') {
        DAT_10004eac[param_1] = cVar1;
        iVar2 = 1;
        if (cVar1 == '\x01') {
          iVar2 = FUN_1000376c(param_1);
        }
      }
    }
  }
  return iVar2;
}



/* ---------------------------------------------------------------- */
/* FUN_10004eb0 @ 10004eb0  (110 bytes) */
/* ---------------------------------------------------------------- */

bool FUN_10004eb0(int param_1,int param_2)

{
  byte bVar1;
  uint uVar2;
  bool bVar3;
  
  uVar2 = *(ushort *)(param_1 + 4) & 0x7ff;
  bVar1 = *(byte *)(param_1 + 3) & 3;
  if (bVar1 == 2) {
    if (param_2 == 2) {
      bVar3 = uVar2 + DAT_10004f28 == 0;
    }
    else {
      bVar3 = uVar2 < 0x41;
    }
  }
  else if (bVar1 == 3) {
    bVar3 = uVar2 <= (-(uint)(param_2 != 2) & DAT_10004f24) + 0x400;
  }
  else {
    bVar3 = false;
    if (bVar1 == 1) {
      bVar3 = uVar2 <= (uint)(param_2 == 2) + DAT_10004f20;
    }
  }
  return bVar3;
}



/* ---------------------------------------------------------------- */
/* FUN_10004f2c @ 10004f2c  (50 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10004f2c(int param_1,byte *param_2,int param_3,undefined1 param_4)

{
  byte *pbVar1;
  
  pbVar1 = param_2 + param_3;
  do {
    if (pbVar1 <= param_2) {
      return;
    }
    while (param_2[1] == 5) {
      *(undefined1 *)(param_1 + (param_2[2] & 0x7f) * 2 + (uint)(param_2[2] >> 7)) = param_4;
      param_2 = param_2 + *param_2;
      if (pbVar1 <= param_2) {
        return;
      }
    }
    param_2 = param_2 + *param_2;
  } while( true );
}



/* ---------------------------------------------------------------- */
/* FUN_10004f60 @ 10004f60  (154 bytes) */
/* ---------------------------------------------------------------- */

undefined4 FUN_10004f60(int *param_1,undefined4 param_2)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  undefined4 uVar5;
  int iVar6;
  
  if (param_1[4] != 0) {
    FUN_100052a8(param_1[4],0xffffffff);
  }
  uVar2 = (uint)*(ushort *)(param_1 + 1);
  uVar1 = (uint)*(ushort *)(param_1 + 2);
  uVar3 = (uint)*(ushort *)((int)param_1 + 10);
  if (*(ushort *)(param_1 + 2) < *(ushort *)((int)param_1 + 10)) {
    iVar6 = uVar2 * 2 - uVar3;
  }
  else {
    iVar6 = -uVar3;
  }
  uVar4 = uVar1 + iVar6 & 0xffff;
  if (uVar4 == 0) {
    uVar3 = (uint)*(ushort *)((int)param_1 + 10);
    uVar5 = 0;
LAB_10004fe0:
    uVar2 = uVar2 * 2;
    if (uVar3 < uVar2) goto LAB_10004fcc;
  }
  else {
    if (uVar2 < uVar4) {
      if (uVar1 < uVar2) {
        uVar1 = uVar1 + uVar2;
        *(short *)((int)param_1 + 10) = (short)uVar1;
      }
      else {
        uVar1 = uVar1 - uVar2;
        *(short *)((int)param_1 + 10) = (short)uVar1;
      }
      uVar3 = uVar1 & 0xffff;
    }
    for (; uVar2 <= uVar3; uVar3 = uVar3 - uVar2 & 0xffff) {
    }
    FUN_10002e1c(param_2,*param_1 + (*(ushort *)((int)param_1 + 6) & 0x7fff) * uVar3);
    uVar5 = 1;
    uVar3 = *(ushort *)((int)param_1 + 10) + 1 & 0xffff;
    uVar2 = (uint)*(ushort *)(param_1 + 1);
    if (*(ushort *)((int)param_1 + 10) <= uVar3) goto LAB_10004fe0;
    uVar3 = 0;
    uVar2 = uVar2 << 1;
  }
  uVar3 = uVar3 - uVar2 & 0xffff;
LAB_10004fcc:
  *(short *)((int)param_1 + 10) = (short)uVar3;
  if (param_1[4] != 0) {
    FUN_10005248();
  }
  return uVar5;
}



/* ---------------------------------------------------------------- */
/* FUN_100050a0 @ 100050a0  (62 bytes) */
/* ---------------------------------------------------------------- */

undefined4 FUN_100050a0(int param_1)

{
  if (*(int *)(param_1 + 0xc) != 0) {
    FUN_100052a8(*(int *)(param_1 + 0xc),0xffffffff);
  }
  if (*(int *)(param_1 + 0x10) != 0) {
    FUN_100052a8(*(int *)(param_1 + 0x10),0xffffffff);
  }
  *(undefined2 *)(param_1 + 10) = 0;
  *(undefined2 *)(param_1 + 8) = 0;
  if (*(int *)(param_1 + 0xc) != 0) {
    FUN_10005248();
  }
  if (*(int *)(param_1 + 0x10) != 0) {
    FUN_10005248();
  }
  return 1;
}



/* ---------------------------------------------------------------- */
/* FUN_100050e0 @ 100050e0  (20 bytes) */
/* ---------------------------------------------------------------- */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_100050e0(void)

{
  FUN_100016b8(0x19);
  _DAT_d0000024 = 0x2000000;
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_100050f4 @ 100050f4  (10 bytes) */
/* ---------------------------------------------------------------- */

void FUN_100050f4(void)

{
  FUN_10003130(DAT_10005100);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10005104 @ 10005104  (196 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10005104(undefined4 *param_1,undefined4 *param_2,uint param_3)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  uint uVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 *puVar6;
  undefined4 *puVar7;
  uint uVar8;
  bool bVar9;
  
  if ((param_2 < param_1) && (param_1 < (undefined4 *)((int)param_2 + param_3))) {
    while (param_3 != 0) {
      param_3 = param_3 - 1;
      *(undefined1 *)((int)param_1 + param_3) = *(undefined1 *)((int)param_2 + param_3);
    }
    return;
  }
  puVar1 = param_2;
  uVar3 = param_3;
  puVar6 = param_1;
  if (0xf < param_3) {
    if ((((uint)param_1 | (uint)param_2) & 3) != 0) goto LAB_10005138;
    uVar8 = param_3 - 0x10 & 0xfffffff0;
    puVar6 = (undefined4 *)((int)param_1 + uVar8) + 4;
    puVar1 = param_1;
    puVar2 = param_2;
    do {
      *puVar1 = *puVar2;
      puVar1[1] = puVar2[1];
      puVar1[2] = puVar2[2];
      puVar7 = puVar2 + 3;
      puVar2 = puVar2 + 4;
      puVar1[3] = *puVar7;
      bVar9 = puVar1 != (undefined4 *)((int)param_1 + uVar8);
      puVar1 = puVar1 + 4;
    } while (bVar9);
    puVar1 = (undefined4 *)((int)param_2 + uVar8 + 0x10);
    uVar3 = param_3 & 0xf;
    if ((param_3 & 0xc) != 0) {
      uVar3 = (param_3 & 0xf) - 4 & 0xfffffffc;
      puVar2 = puVar1;
      do {
        uVar5 = *puVar2;
        puVar7 = (undefined4 *)((int)puVar2 + ((int)param_1 - (int)param_2));
        puVar2 = puVar2 + 1;
        *puVar7 = uVar5;
      } while (puVar2 != (undefined4 *)((int)param_2 + uVar3 + uVar8 + 0x14));
      iVar4 = uVar3 + 4;
      puVar1 = (undefined4 *)(iVar4 + (int)puVar1);
      uVar3 = param_3 & 3;
      puVar6 = (undefined4 *)(iVar4 + (int)puVar6);
    }
  }
  param_2 = puVar1;
  param_1 = puVar6;
  param_3 = uVar3;
  if (uVar3 == 0) {
    return;
  }
LAB_10005138:
  iVar4 = 0;
  do {
    *(undefined1 *)((int)param_1 + iVar4) = *(undefined1 *)((int)param_2 + iVar4);
    bVar9 = param_3 - 1 != iVar4;
    iVar4 = iVar4 + 1;
  } while (bVar9);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_100051c8 @ 100051c8  (84 bytes) */
/* ---------------------------------------------------------------- */

int FUN_100051c8(uint *param_1)

{
  uint uVar1;
  uint *puVar2;
  uint *puVar3;
  
  for (puVar2 = param_1; ((uint)puVar2 & 3) != 0; puVar2 = (uint *)((int)puVar2 + 1)) {
    if ((char)*puVar2 == '\0') goto LAB_100051e0;
  }
  uVar1 = *puVar2 + DAT_1000521c & ~*puVar2;
  puVar3 = puVar2;
  while ((puVar2 = puVar3, (uVar1 & DAT_10005220) == 0 &&
         (puVar2 = puVar3 + 1, (puVar3[1] + DAT_1000521c & ~puVar3[1] & DAT_10005220) == 0))) {
    puVar2 = puVar3 + 2;
    puVar3 = puVar3 + 2;
    uVar1 = *puVar2 + DAT_1000521c & ~*puVar2;
  }
  for (; (char)*puVar2 != '\0'; puVar2 = (uint *)((int)puVar2 + 1)) {
  }
LAB_100051e0:
  return (int)puVar2 - (int)param_1;
}



/* ---------------------------------------------------------------- */
/* FUN_10005228 @ 10005228  (10 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10005228(undefined4 param_1)

{
                    /* WARNING: Could not recover jumptable at 0x10005230. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*DAT_10005234)(param_1);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10005238 @ 10005238  (10 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10005238(undefined4 param_1)

{
                    /* WARNING: Could not recover jumptable at 0x10005240. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*DAT_10005244)(param_1);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10005248 @ 10005248  (10 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10005248(undefined4 param_1)

{
                    /* WARNING: Could not recover jumptable at 0x10005250. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*DAT_10005254)(param_1);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10005258 @ 10005258  (10 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10005258(undefined4 param_1)

{
                    /* WARNING: Could not recover jumptable at 0x10005260. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*DAT_10005264)(param_1);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10005268 @ 10005268  (10 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10005268(undefined4 param_1)

{
                    /* WARNING: Could not recover jumptable at 0x10005270. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*DAT_10005274)(param_1);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10005278 @ 10005278  (10 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10005278(undefined4 param_1)

{
                    /* WARNING: Could not recover jumptable at 0x10005280. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*DAT_10005284)(param_1);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10005288 @ 10005288  (10 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10005288(undefined4 param_1)

{
                    /* WARNING: Could not recover jumptable at 0x10005290. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*DAT_10005294)(param_1);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10005298 @ 10005298  (10 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10005298(undefined4 param_1)

{
                    /* WARNING: Could not recover jumptable at 0x100052a0. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*DAT_100052a4)(param_1);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_100052a8 @ 100052a8  (10 bytes) */
/* ---------------------------------------------------------------- */

void FUN_100052a8(undefined4 param_1)

{
                    /* WARNING: Could not recover jumptable at 0x100052b0. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*DAT_100052b4)(param_1);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_100052b8 @ 100052b8  (10 bytes) */
/* ---------------------------------------------------------------- */

void FUN_100052b8(undefined4 param_1)

{
                    /* WARNING: Could not recover jumptable at 0x100052c0. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*DAT_100052c4)(param_1);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_100052c8 @ 100052c8  (10 bytes) */
/* ---------------------------------------------------------------- */

void FUN_100052c8(undefined4 param_1)

{
                    /* WARNING: Could not recover jumptable at 0x100052d0. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*DAT_100052d4)(param_1);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_100052d8 @ 100052d8  (10 bytes) */
/* ---------------------------------------------------------------- */

void FUN_100052d8(undefined4 param_1)

{
                    /* WARNING: Could not recover jumptable at 0x100052e0. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*DAT_100052e4)(param_1);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_100052e8 @ 100052e8  (10 bytes) */
/* ---------------------------------------------------------------- */

void FUN_100052e8(undefined4 param_1)

{
                    /* WARNING: Could not recover jumptable at 0x100052f0. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*DAT_100052f4)(param_1);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10005718 @ 10005718  (152 bytes) */
/* ---------------------------------------------------------------- */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4
FUN_10005718(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  char cVar1;
  bool bVar2;
  undefined4 uVar3;
  int iVar4;
  uint uVar5;
  undefined4 *puVar6;
  char cVar7;
  
  uVar5 = 0;
  bVar2 = (bool)isCurrentModePrivileged();
  if (bVar2) {
    uVar5 = isIRQinterruptsEnabled();
  }
  disableIRQinterrupts();
  do {
  } while (*(int *)*param_1 == 0);
  DataMemoryBarrier(0x1f);
  cVar1 = *(char *)(param_1 + 1);
  puVar6 = (undefined4 *)*param_1;
  if (cVar1 < '\0') {
    cVar7 = (char)_DAT_d0000000;
LAB_1000579e:
    *(char *)(param_1 + 1) = cVar7;
    DataMemoryBarrier(0x1f);
    *puVar6 = 0;
    bVar2 = (bool)isCurrentModePrivileged();
    if (bVar2) {
      enableIRQinterrupts((uVar5 & 1) == 1);
    }
    uVar3 = 1;
  }
  else {
    DataMemoryBarrier(0x1f);
    *puVar6 = 0;
    bVar2 = (bool)isCurrentModePrivileged();
    if (bVar2) {
      enableIRQinterrupts((uVar5 & 1) == 1);
    }
    cVar7 = DAT_d0000000;
    if (DAT_d0000000 != cVar1) {
      do {
        uVar5 = 0;
        bVar2 = (bool)isCurrentModePrivileged();
        if (bVar2) {
          uVar5 = isIRQinterruptsEnabled();
        }
        disableIRQinterrupts();
        do {
        } while (*(int *)*param_1 == 0);
        DataMemoryBarrier(0x1f);
        puVar6 = (undefined4 *)*param_1;
        if (0x7f < *(byte *)(param_1 + 1)) goto LAB_1000579e;
        DataMemoryBarrier(0x1f);
        *puVar6 = 0;
        bVar2 = (bool)isCurrentModePrivileged();
        if (bVar2) {
          enableIRQinterrupts((uVar5 & 1) == 1);
        }
        iVar4 = FUN_100066b8(param_3,param_4);
      } while (iVar4 == 0);
    }
    uVar3 = 0;
  }
  return uVar3;
}



/* ---------------------------------------------------------------- */
/* FUN_1000628c @ 1000628c  (154 bytes) */
/* ---------------------------------------------------------------- */

void FUN_1000628c(int param_1,int param_2)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  undefined1 *puVar5;
  uint uVar6;
  undefined1 *puVar7;
  uint uVar8;
  
  uVar2 = **(uint **)(param_1 + 8);
  if (param_2 != 0) {
    uVar2 = uVar2 >> 0x10;
  }
  uVar1 = uVar2 & 0x3ff;
  if (*(char *)(param_1 + 1) == '\0') {
    *(short *)(param_1 + 0x16) = (short)uVar1 + *(short *)(param_1 + 0x16);
  }
  else {
    uVar4 = *(uint *)(param_1 + 0x10);
    uVar6 = *(int *)(param_1 + 0xc) + param_2 * 0x40;
    uVar8 = uVar1 - 1;
    if (uVar1 != 0) {
      if (((uVar8 < 7) || (uVar4 - (*(int *)(param_1 + 0xc) + param_2 * 0x40 + 1) < 3)) ||
         (((uVar4 | uVar6) & 3) != 0)) {
        uVar2 = 0;
        do {
          *(undefined1 *)(uVar4 + uVar2) = *(undefined1 *)(uVar6 + uVar2);
          uVar2 = uVar2 + 1;
        } while (uVar1 != uVar2);
        uVar4 = *(uint *)(param_1 + 0x10);
      }
      else {
        uVar3 = 0;
        uVar2 = uVar2 & 0x3fc;
        do {
          *(undefined4 *)(uVar4 + uVar3) = *(undefined4 *)(uVar6 + uVar3);
          uVar3 = uVar3 + 4;
        } while (uVar2 != uVar3);
        puVar5 = (undefined1 *)(uVar4 + uVar2);
        puVar7 = (undefined1 *)(uVar6 + uVar2);
        if (((uVar1 != uVar2) && (*puVar5 = *puVar7, uVar8 != uVar2)) &&
           (puVar5[1] = puVar7[1], uVar8 - uVar2 != 1)) {
          puVar5[2] = puVar7[2];
        }
        uVar4 = *(uint *)(param_1 + 0x10);
      }
    }
    *(short *)(param_1 + 0x16) = (short)uVar1 + *(short *)(param_1 + 0x16);
    *(uint *)(param_1 + 0x10) = uVar4 + uVar1;
  }
  if (uVar1 < *(ushort *)(param_1 + 0x18)) {
    *(undefined2 *)(param_1 + 0x14) = 0;
  }
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_100063b8 @ 100063b8  (68 bytes) */
/* ---------------------------------------------------------------- */

/* WARNING: Control flow encountered bad instruction data */

void FUN_100063b8(int param_1,uint param_2,uint param_3)

{
  uint uVar1;
  uint *puVar2;
  uint uVar3;
  bool bVar4;
  
  puVar2 = *(uint **)(param_1 + 8);
  uVar1 = 0;
  if (param_2 != 0) {
    uVar1 = param_2 & *puVar2;
  }
  if ((param_3 != 0) && (uVar1 = uVar1 | param_3, (param_3 & 0x400) != 0)) {
    if ((*puVar2 & 0x400) != 0) {
      FUN_10006708(DAT_10006404,*(undefined1 *)(param_1 + 2));
                    /* WARNING: Bad instruction - Truncating control flow here */
      halt_baddata();
    }
    *puVar2 = DAT_100063fc & uVar1;
    if (-1 < *(int *)(DAT_10006400 + 0x40) << 0x1e) {
      uVar3 = 0xc;
      do {
        bVar4 = 2 < uVar3;
        uVar3 = uVar3 - 3;
      } while (bVar4);
      puVar2 = *(uint **)(param_1 + 8);
    }
  }
  *puVar2 = uVar1;
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_100066b8 @ 100066b8  (10 bytes) */
/* ---------------------------------------------------------------- */

void FUN_100066b8(undefined4 param_1)

{
                    /* WARNING: Could not recover jumptable at 0x100066c0. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*DAT_100066c4)(param_1);
  return;
}



/* ---------------------------------------------------------------- */
/* FUN_10006708 @ 10006708  (10 bytes) */
/* ---------------------------------------------------------------- */

void FUN_10006708(undefined4 param_1)

{
  (*DAT_10006714)(param_1);
  return;
}



/* TOTAL: 155 functions, decompiled ok=155 failed=0 */
