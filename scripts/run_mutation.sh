#!/bin/bash

#SBATCH --chdir=/cluster-home/dario.barone/TravelAndMutate
#SBATCH --error=/beegfs/labpoletto/dario/logs/errors/%x_%A_%a.err
#SBATCH --output=/beegfs/labpoletto/dario/logs/output/%x_%A_%a.out
#SBATCH --partition=base
#SBATCH --no-requeue
#SBATCH --mem-per-cpu=8G
#SBATCH --cpus-per-task=1

#SBATCH --job-name=betaMrmeanMrk
#SBATCH --array=15,16,17,18,19,35,36,37,38,39,55,56,57,58,59,75,76,77,78,79

apptainer exec /beegfs/labpoletto/sif/python.sif python scripts/run_mutation.py --dir data/ --name betaMrmeanMrk --group $SLURM_ARRAY_TASK_ID --seed 0