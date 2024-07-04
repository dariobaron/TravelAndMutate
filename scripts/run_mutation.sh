#!/bin/bash

#SBATCH --chdir=/cluster-home/dario.barone/TravelAndMutate
#SBATCH --error=/beegfs/labpoletto/dario/logs/errors/%x_%A_%a.err
#SBATCH --output=/beegfs/labpoletto/dario/logs/output/%x_%A_%a.out
#SBATCH --partition=base
#SBATCH --no-requeue
#SBATCH --mem-per-cpu=8G
#SBATCH --cpus-per-task=1

#SBATCH --job-name=newfitness
#SBATCH --array=0-599

group=$((SLURM_ARRAY_TASK_ID%60))
seed=$((SLURM_ARRAY_TASK_ID/60))

python scripts/run_mutation.py --dir data/ --name newfitness --group $group --seed $seed