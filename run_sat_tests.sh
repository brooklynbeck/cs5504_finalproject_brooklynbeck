#!/bin/bash
set -euo pipefail

#updated script from hw9 with new architecture settings and workloads
#for computer architecture final project
#updates by Brooklyn Beck

GEM5_BIN="gem5/build/ALL/gem5.opt"
SE_PY="gem5/configs/deprecated/example/se.py"

OUT_ROOT="stats_satellite_architectures"

#default settings
CPU_TYPE="X86O3CPU"
NUM_CPUS=4
L1D_SIZE="32KiB"
L1I_SIZE="32KiB"
L2_SIZE="1MiB"
L3_SIZE="4MiB"
frequency="2.4GHz"
memory='DDR4_2400_8x8'
architecture="unspecified"
MEM_SIZE="8GiB"

PROTOCOLS=(
  "MESI_Two_Level"
  "CHI"
)

WORKLOADS=(
  "dense"
  "independent_writes"
#  "yolo" #ended up not having enough space in rlogin for yolo, pivot to include dense to have comparison with another non parallel work
  "gerry"
)

mkdir -p sat_stats
mkdir -p "${OUT_ROOT}"
i=1
for protocol in "${PROTOCOLS[@]}"; do
  for workload in "${WORKLOADS[@]}"; do
    #select architecture settings
    if [ "$i" -eq 1 ]; then #raspberry pi settings
      CPU_TYPE="X86O3CPU"
      NUMCPUS=4
      L1D_SIZE="32KiB"
      L1I_SIZE="32KiB"
      L2_SIZE="512KiB"
      L3_SIZE="4MiB"
      frequency="2.4GHz"
      memory='DDR4_2400_8x8'
      architecture="rpi5"
    elif [ "$i" -eq 2 ]; then #jetson orin nano settings (tbd)
      CPU_TYPE="X86O3CPU"
      NUM_CPUS=6
      L1D_SIZE="64KiB"
      L1I_SIZE="64KiB"
      L2_SIZE="2MiB"
      L3_SIZE="4MiB"
      frequency="1500MHz"
      memory='LPDDR5_6400_1x16_BG_BL16'
      architecture="jetson"
    else #default settings
      CPU_TYPE="X86O3CPU"
      NUM_CPUS=4
      L1D_SIZE="32KiB"
      L1I_SIZE="32KiB"
      L2_SIZE="1MiB"
      L3_SIZE="4MiB"
      frequency="2.4GHz"
      memory='DDR4_2400_8x8'
      architecture="unspecified"
    fi

    #outdir="sat_stats"
    outdir="${OUT_ROOT}/${architecture}/${workload}"
    mkdir -p "${outdir}"

    echo "=================================================="
    echo "Architecture : ${architecture}"
    echo "Workload : ${workload}"
    echo "Out dir  : ${outdir}"
    echo "=================================================="

    "${GEM5_BIN}" \
      -d "${outdir}" \
      "${SE_PY}" \
      --ruby \
      --protocol="${protocol}" \
      --cpu-type="${CPU_TYPE}" \
      --num-cpus="${NUM_CPUS}" \
      --l1d_size="${L1D_SIZE}" \
      --l1i_size="${L1I_SIZE}" \
      --l2_size="${L2_SIZE}" \
      --l3_size="${L3_SIZE}" \
      --mem-type="$memory" \
      --mem-size="${MEM_SIZE}" \
      --cpu-clock="${frequency}" \
      --cmd="./${workload}"

    cp ${outdir}/stats.txt sat_stats/stats_${architecture}_${workload}.txt

  done
  i=$((i+1))
done

echo "All runs completed."

