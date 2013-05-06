# RUN: llvm-mc -triple s390x-linux-gnu -show-encoding %s | FileCheck %s

#CHECK: clfhsi	0, 0                    # encoding: [0xe5,0x5d,0x00,0x00,0x00,0x00]
#CHECK: clfhsi	4095, 0                 # encoding: [0xe5,0x5d,0x0f,0xff,0x00,0x00]
#CHECK: clfhsi	0, 65535                # encoding: [0xe5,0x5d,0x00,0x00,0xff,0xff]
#CHECK: clfhsi	0(%r1), 42              # encoding: [0xe5,0x5d,0x10,0x00,0x00,0x2a]
#CHECK: clfhsi	0(%r15), 42             # encoding: [0xe5,0x5d,0xf0,0x00,0x00,0x2a]
#CHECK: clfhsi	4095(%r1), 42           # encoding: [0xe5,0x5d,0x1f,0xff,0x00,0x2a]
#CHECK: clfhsi	4095(%r15), 42          # encoding: [0xe5,0x5d,0xff,0xff,0x00,0x2a]

	clfhsi	0, 0
	clfhsi	4095, 0
	clfhsi	0, 65535
	clfhsi	0(%r1), 42
	clfhsi	0(%r15), 42
	clfhsi	4095(%r1), 42
	clfhsi	4095(%r15), 42
