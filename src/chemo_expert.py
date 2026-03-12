import numpy as np
# Cria o atalho que o MXNet procura
np.bool = bool 
np.float = float
np.int = int

import pandas as pd
import mxnet as mx
from mxnet import nd

def process_chemistry():
    df = pd.read_parquet('data/silver/part_B_chemistry.parquet')
    
    # Transforma os dados químicos em Tensores do MXNet (Acelerado por MKL)
    chem_tensor = nd.array(df[['glutamate', 'gaba', 'drug_affinity']].values)
    
    # Simula uma operação de 'Ativação Farmacológica' (Função não-linear)
    # Aqui o modelo 'aprende' a saturação química de cada área
    activation = nd.sigmoid(chem_tensor * 1.5)
    
    # Salva o insight químico
    res = pd.DataFrame(activation.asnumpy(), columns=['glut_act', 'gaba_act', 'drug_res'])
    res['roi_id'] = df['roi_id']
    res.to_parquet('data/gold/chemistry_results.parquet')
    print("✅ Química (MXNet) processada e salva em data/gold/")

if __name__ == "__main__":
    process_chemistry()
