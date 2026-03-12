import pandas as pd
import numpy as np
import os

def create_partitions(n_regions=500):
    # Garante que as pastas existam
    os.makedirs('data/silver', exist_ok=True)
    roi_ids = np.arange(n_regions)

    # PARTIÇÃO A: GEOMETRIA (Arquitetura do Cérebro)
    df_geometry = pd.DataFrame({
        'roi_id': roi_ids,
        'axon_density': np.random.beta(2, 5, n_regions), # Microestrutura física
        'node_degree': np.random.poisson(12, n_regions), # Conectividade teórica
        'cortical_thickness': np.random.normal(2.5, 0.5, n_regions) # Espessura do voxel
    })
    df_geometry.to_parquet('data/silver/part_A_geometry.parquet')

    # PARTIÇÃO B: QUÍMICA (O Combustível Sináptico)
    df_chemistry = pd.DataFrame({
        'roi_id': roi_ids,
        'glutamate': np.random.uniform(0.2, 0.8, n_regions),
        'gaba': np.random.uniform(0.1, 0.4, n_regions),
        'drug_affinity': np.random.exponential(0.3, n_regions), # Reação ao fármaco
        'drug_affinity': np.random.exponential(0.5, n_regions),
        'drug_decay_rate': np.random.uniform(0.01, 0.1, n_regions),
        'receptor_saturation': np.random.beta(2, 2,n_regions)
    })
    df_chemistry.to_parquet('data/silver/part_B_chemistry.parquet')

    # PARTIÇÃO C: BIOFÍSICA (O Fluxo Elétrico)
    df_biophysics = pd.DataFrame({
        'roi_id': roi_ids,
        'membrane_potential': np.random.normal(-70, 2, n_regions),
        'electron_drift': np.random.uniform(0.9, 1.1, n_regions),
        'synaptic_efficiency': np.random.uniform(0.5, 0.95, n_regions)
    })
    df_biophysics.to_parquet('data/silver/part_C_biophysics.parquet')
    
    print("✅ Partições A, B e C geradas em data/silver/")

if __name__ == "__main__":
    create_partitions()

