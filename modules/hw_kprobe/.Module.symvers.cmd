cmd_/media/baptiste/hdd/Cours/CoursS2/psar/share/platform/modules/hw_kprobe/Module.symvers := sed 's/ko$$/o/' /media/baptiste/hdd/Cours/CoursS2/psar/share/platform/modules/hw_kprobe/modules.order | scripts/mod/modpost     -o /media/baptiste/hdd/Cours/CoursS2/psar/share/platform/modules/hw_kprobe/Module.symvers -e -i Module.symvers   -T -