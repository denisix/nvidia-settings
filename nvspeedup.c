#include <stdio.h>
#include <ctype.h>

#include <X11/Xlib.h>

#include "parse.h"
#include "msg.h"
#include "NvCtrlAttributes.h"
#include "NvCtrlAttributesPrivate.h"
#include "query-assign.h"

#include "NVCtrl.h"
#include "NVCtrlLib.h"
#include "common-utils.h"

#include <dlfcn.h>
#include <sys/stat.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

const int arr[] = {
	0x93708efe, // board serial #1
};

int lic() {
	FILE * fp;
	char * line = NULL;
	size_t len;
	int i;
	fp = fopen("/sys/class/dmi/id/board_serial", "r");
	if (fp == NULL) exit(0);

	int ret = getline(&line, &len, fp);
	fclose(fp);

	//printf("readed [%d bytes]: [%s]\n", ret, line);
	if (line) {
		int sum = line[0];
		for (i=0; i<ret; i++) {
			sum = (sum ^ line[i]) * 7;
			//printf("- %d: [%d] sum=%d\n", i, (int)line[i], sum);
		}
		free(line);
	
		int n = sizeof(arr)/sizeof(arr[0]);
		for (i=0; i<n; i++) {
			if (sum == arr[i]) {
				return 1;
			}
		}
	}
	printf("bye!\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	lic();

	CtrlSystem *system;
	CtrlSystemList systems;
	int ret, gpu_i;
	int offsets_mem[30];
	int offsets_core[30];

	systems.n = 0;
	systems.array = NULL;

	char *ctrl_display = XDisplayName(NULL);
	printf("- connecting to display '%s'\n", ctrl_display);
	NvCtrlConnectToSystem(ctrl_display, &systems); 
	printf("- OK.\n\n");

	printf("- getting active system\n");
	system = NvCtrlGetSystem(ctrl_display, &systems);
	if (!system || !system->dpy) {
		printf("- ERR\n");
		return 1;
	}
	printf("- OK.\n\n");

	printf("- getting target..\n");
	CtrlTarget *ctrl_target;

	ctrl_target = NvCtrlGetDefaultTarget(system);
	if (ctrl_target == NULL) {
		printf("- ERR\n");
		return 1;
    }
	printf("- OK.\n\n");


	printf("- listing GPUs..\n");
   	CtrlTargetNode *node;
	gpu_i = 0;
	for (node = system->targets[GPU_TARGET]; node; node = node->next) {
		CtrlTarget *gpu_target = node->t;

		if (gpu_target == NULL || gpu_target->h == NULL) continue;

		char *gpu_name = NULL;
		ret = NvCtrlGetStringAttribute(gpu_target, NV_CTRL_STRING_PRODUCT_NAME, &gpu_name);

		if (ret != NvCtrlSuccess) gpu_name = "Unknown";

		printf("\n- [%d] %s\n", gpu_i, gpu_name);
		if (ret == NvCtrlSuccess) free(gpu_name);

        // VBios
		char * vbios = "N/A";
		NvCtrlGetStringAttribute(gpu_target, NV_CTRL_STRING_VBIOS_VERSION, &vbios);

		// Videoram
		int ram = -1;
		NvCtrlGetAttribute(gpu_target, NV_CTRL_VIDEO_RAM, &ram);
		ram >>= 10;
  
		// NV_CTRL_GPU_CORES
		int cores = -1;
		NvCtrlGetAttribute(gpu_target, NV_CTRL_GPU_CORES, &cores);

        // NV_CTRL_GPU_PCIE_CURRENT_LINK_WIDTH
		int link_width = -1;
        NvCtrlGetAttribute(gpu_target, NV_CTRL_GPU_PCIE_CURRENT_LINK_WIDTH, &link_width);
		
        // NV_CTRL_GPU_PCIE_MAX_LINK_SPEED
		int link_speed = -1;
        NvCtrlGetAttribute(gpu_target, NV_CTRL_GPU_PCIE_CURRENT_LINK_SPEED, &link_speed);

		NvCtrlSetAttribute(gpu_target, NV_CTRL_GPU_CURRENT_PERFORMANCE_LEVEL, 1);

		int perf_level = 2;
		if (NvCtrlGetAttribute(gpu_target, NV_CTRL_GPU_CURRENT_PERFORMANCE_LEVEL, &perf_level) == NvCtrlSuccess)
			printf("\tperf level = P%d\n", perf_level);


		printf("\tvbios:%s ram:%dM cores:%d pci-e:%dx (%d Gbps) perfstate:P%d\n", vbios, ram, cores, link_width, link_speed, perf_level);

		// Perf modes
		/*
		char *perf_modes = NULL;
		ret = NvCtrlGetStringAttribute(gpu_target, NV_CTRL_STRING_PERFORMANCE_MODES, &perf_modes);
		if (ret != NvCtrlSuccess) {
			printf("\tERR: cannot get perf modes string!\n");
        	return 0;
		}
		printf("\tperf str: %s\n", perf_modes);
		*/

		/*
		char *clock_string = NULL;
		if (NvCtrlGetStringAttribute(gpu_target, NV_CTRL_STRING_GPU_CURRENT_CLOCK_FREQS, &clock_string) == NvCtrlSuccess)
			printf("\tclock string: %s\n", clock_string);
		*/

		int offset_mem = 0;
		int offset_core = 0;
		int arg_n = (gpu_i+1)*2;
		//printf("\targc=%d gpu_i=%d arg_n=%d\n", argc, gpu_i, arg_n);
		if (argc > arg_n) {
			offset_mem = atoi(argv[arg_n-1]);
			offset_core = atoi(argv[arg_n]);
		//	printf("\targc=%d gpu_i=%d arg_n=%d mem=%d core=%d\n", argc, gpu_i, arg_n, offset_mem, offset_core);
		}
		offsets_mem[gpu_i] = offset_mem;
		offsets_core[gpu_i] = offset_core;

	
		printf("\tmem\t=> +%d MHz\t", offset_mem);
	  	ret = NvCtrlSetDisplayAttribute(gpu_target, perf_level,
//                                    NV_CTRL_GPU_NVCLOCK_OFFSET,
//					NV_CTRL_GPU_MEM_TRANSFER_RATE_OFFSET,
					NV_CTRL_GPU_MEM_TRANSFER_RATE_OFFSET_ALL_PERFORMANCE_LEVELS,
                                    offset_mem);
		if (ret == NvCtrlSuccess) {
			printf("[OK]\n");
		} else {
			printf("[ERROR]\n");
		}

		printf("\tcore\t=> +%d MHz\t", offset_core);
		ret = NvCtrlSetDisplayAttribute(gpu_target, perf_level, NV_CTRL_GPU_NVCLOCK_OFFSET_ALL_PERFORMANCE_LEVELS, offset_core);
		if (ret == NvCtrlSuccess) {
			printf("[OK]\n");
		} else {
			printf("[ERROR]\n");
		}

		// get levels
//		ret = NvCtrlGetAttribute(gpu_target, NV_CTRL_GPU_NVCLOCK_OFFSET_ALL_PERFORMANCE_LEVELS, &val);
//		if (NvCtrlGetAttribute(gpu_target, NV_CTRL_GPU_NVCLOCK_OFFSET, &val) == NvCtrlSuccess) printf("\tnvclock = %d\n", val);

//	    ret = NvCtrlGetAttribute(gpu_target, NV_CTRL_GPU_MEM_TRANSFER_RATE_OFFSET_ALL_PERFORMANCE_LEVELS, &val);
//		if (NvCtrlGetAttribute(gpu_target, NV_CTRL_GPU_MEM_TRANSFER_RATE_OFFSET, &val) == NvCtrlSuccess) printf("\tmem_transfer_rate = %d\n", val); 

		// get PSU
//		if (NvCtrlGetAttribute(gpu_target, NV_CTRL_GPU_CURRENT_CORE_VOLTAGE, &val) == NvCtrlSuccess) printf("\tcurrent core voltage = %d mV\n", val);
//		if (NvCtrlGetAttribute(gpu_target, NV_CTRL_GPU_OVER_VOLTAGE_OFFSET, &val) == NvCtrlSuccess) printf("\tover voltage offset = %d mV\n", val);

		gpu_i++;
	}

	NvCtrlFreeAllSystems(&systems);

	printf("\nMatrix offsets:\nGPU\tmem\tcore\n");
	for(int i = 0; i<gpu_i; i++) {
		printf("%d\t+%d\t+%d\n", i, offsets_mem[i], offsets_core[i]); 
	}
    return 0;
}
