#!/bin/bash

#SBATCH --chdir=/beegfs/labpoletto/dario/TravelAndMutate
#SBATCH --error=/beegfs/labpoletto/dario/logs/errors/%x_%A_%a.err
#SBATCH --output=/beegfs/labpoletto/dario/logs/output/%x_%A_%a.out
#SBATCH --partition=base
#SBATCH --no-requeue
#SBATCH --mem-per-cpu=2G
#SBATCH --cpus-per-task=1

#SBATCH --job-name=fitness
#SBATCH --array=61-239

#group=$((SLURM_ARRAY_TASK_ID%60))
#seed=$((SLURM_ARRAY_TASK_ID/60))

python scripts/multi_run_mutation.py --dir data/ --name fitness --group $SLURM_ARRAY_TASK_ID --nsucc 5 --nprocs 0