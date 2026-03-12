import pandas as pd
import networkx as nx
from GraphRicciCurvature.OllivierRicci import OllivierRicci

def process_geometry():
    # Lê a partição específica
    df = pd.read_parquet('data/silver/part_A_geometry.parquet')
    
    # Simula um Grafo de Conectividade entre as regiões (ROIs)
    G = nx.erdos_renyi_graph(n=len(df), p=0.05)
    
    # Adiciona os pesos da 'microestrutura' (axon_density) nas arestas
    for u, v in G.edges():
        G[u][v]['weight'] = df.iloc[u]['axon_density'] + df.iloc[v]['axon_density']
    
    # Calcula a Curvatura de Ricci (Onde a informação 'engasga')
    orc = OllivierRicci(G, alpha=0.5)
    orc.compute_ricci_curvature()
    
    # Salva o resultado preliminar para a união na Gold
    # Extraímos a curvatura de cada nó para o diagnóstico
    ricci_data = pd.DataFrame.from_dict(dict(orc.G.nodes(data='ricciCurvature')), 
                                       orient='index', columns=['ricci_curv'])
    ricci_data.to_parquet('data/gold/geometry_results.parquet')
    print("✅ Geometria (Ricci) processada e salva em data/gold/")

if __name__ == "__main__":
    process_geometry()

