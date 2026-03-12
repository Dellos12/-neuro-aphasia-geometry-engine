import pandas as pd
import numpy as np

def run_diagnostic_differencial():
    print("🏣 Iniciando Diagnóstico Diferencial: Química vs. Elétrica...")
    
    # 1. Carregar o Catálogo Consolidado (A Literatura de Vetores)
    try:
        catalog = pd.read_parquet('data/gold/final_aphasia_catalog.parquet')
    except FileNotFoundError:
        print('🛑 Erro: Catálogo não encontrado> Rode o build_catalogo.parquet') 
        return
    
    # 2. DEFINIÇÃO DE SCORE DE FALHA (Normalização de Sinais)
    # Score Químico: Qanto menor a pesistencia, maio a falh química
    chem_failure_score = 1 / (catalog['drug_persistence'] + 0.1)
    
    # Score Elétrico: Quanto menor a eficiènci sináptica, maior a falha elétrica
    elec_failure_score = 1 / (catalog['synaptic_efficiency'] + 0.1)
    
    # 3. LÓGICA DE CAUSA RAIZ (O Insight do Especialista)
    # Comparamos as duas magnitudes de falha
    catalog['root_cause'] = np.where(
        chem_failure_score > elec_failure_score,
        "FALHA_QUIMICA",
        "FALHA_ELETRICA"
    )
    
    # 4. FILTRO DE MICROESTRUTURA: Apenas para zonas críticas
    # Se não está em zon crítica,  a causa raiz é "ESTAVEL"
    catalog.loc[catalog['is_critical_zone'] == False, 'root_cause'] = "ESTAVEL"
    
    # 5. RESULTADO PONTUAL (A Literatura)
    print("\n Relatório de Diagnóstico po ROI (Zonas Críticas):")
    critical_rois = catalog[catalog['is_critical_zone'] == True]

    view_cols = ['roi_id', 'ricci_curv', 'drug_persistence', 'synaptic_efficiency', 'root_cause']
    print(critical_rois[view_cols].head(10))
    
    # Salvando o Diagnóstico Final
    catalog.to_parquet('data/gold/final_diagnosis.parquet')
    print(f"\n💞 Diagnóstico concluido. {len(critical_rois)} ROIs em estado crítico identificados.")
    
if __name__ == "__main__":
    run_diagnostic_differencial()
    
    