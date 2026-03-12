import pandas as pd
import numpy as np
import os

def build_aphasia_catalog():
    print("🧾 Iniciando a construção do Catálogo de Microstrutura")
    
    # 1. Carregamento das Camadas (Bronze/Silver -> Gold)
    try:
        df_geo = pd.read_parquet('data/gold/geometry_results.parquet')
        df_chemo = pd.read_parquet('data/silver/part_B_chemistry.parquet')
        df_bio = pd.read_parquet('data/silver/part_C_biophysics.parquet')
    except FileNotFoundError:
        print("🚩 Erro: Partições não encontradas. Rode os scripts de especialistas primeiro.")
        return
    
    # 2. O JOIN DE ALTA DENSIDADE (Unificação da Realidade)
    # Alinhamos Geometria (Forma), Química (Comustível) e Biofísica (Elétrons)
    catalog = pd.merge(df_geo, df_chemo, left_index=True, right_on='roi_id')
    catalog = pd.merge(catalog, df_bio, on='roi_id')
    
    # 3. PARÂMETROS DE CINÉTICA (A "Linha de Corte")
    # Calculmos a Persistência Residual do Fármaco (Afinidade / Taxa de Degradação)
    # O +0.01 evita divisão por zero em receptores inativos
    
    catalog['drug_persistence'] = catalog['drug_affinity'] / (catalog['drug_decay_rate'] + 0.01)
    
    # DEFINIÇÃO DO LIMIAR:
    # Um ROI entra em Zona Critica se a Persistência < 4.0 e a Curvatura de Ricci < 0
    threshold_corte = 4.0
    catalog['is_critical_zone'] = (catalog['drug_persistence'] < threshold_corte) & (catalog['ricci_curv'] < 0)
    
    # 4.MÉTRICA DE SEVERIDADE (A Geodesica do Risco)
    # Multiplicamos o colapso geométrico pela fragilidade química
    catalog['aphasia_severity'] = np.abs(catalog['ricci_curv']) * (1 / (catalog['drug_persistence'] + 0.1))
    
    # 5. SALVAMENTO DO CTALOGO CONSOLIDADO
    os.makedirs('data/gold', exist_ok=True)
    catalog.to_parquet('data/gold/final_aphasia_catalog.parquet')
    
    print(f" 🏥 Catálogo consolidade com {len(catalog.columns)} dimensões.")
    print(" Análise de Microestrutura (top 5 ROIs):")
    
    # Exibição formatada para análise do insight
    cols_view = ['roi_id', 'ricci_curv', 'drug_persistence', 'aphasia_severity', 'is_critical_zone']

if __name__=="__main__":
    build_aphasia_catalog()
    
        
        
     
        
