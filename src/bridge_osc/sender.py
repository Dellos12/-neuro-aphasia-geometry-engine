
import pandas as pd
import numpy as np
import umap
from pythonosc import udp_client
import time

class BrainOracleSender:
    def __init__(self, host="127.0.0.1", port=5005):
        self.client = udp_client.SimpleUDPClient(host, port)
        # Carrega a literatura de 0,001% (Causa Raiz)
        self.df = pd.read_parquet('data/gold/final_diagnosis.parquet')
        self.embedding = None

    def compute_geometry(self):
        print("🌀 Projetando Hiperplano UMAP 3D...")
        # Colunas cruciais para a assinatura da Afasia
        features = ['ricci_curv', 'drug_persistence', 'synaptic_efficiency', 'glutamate']
        reducer = umap.UMAP(n_neighbors=20, min_dist=0.1, n_components=3, metric='cosine')
        self.embedding = reducer.fit_transform(self.df[features].values)
        print("✅ Geometria Latente Gerada.")

    def sync_atlas(self):
        print("📡 Estacionando BrainGraph no openFrameworks...")
        cause_map = {"ESTAVEL": 0, "FALHA_QUIMICA": 1, "FALHA_ELETRICA": 2}

        for i, row in self.df.iterrows():
            x, y, z = self.embedding[i]
            cause_id = cause_map.get(row['root_cause'], 0)
            
            # Envia Posição e Causa (/roi/setup é o endereço unificado)
            self.client.send_message("/roi/setup", [
                int(row['roi_id']), float(x), float(y), float(z), int(cause_id)
            ])
            
            # Envia Arestas (Vizinhos mais próximos no espaço latente)
            dist = np.linalg.norm(self.embedding - self.embedding[i], axis=1)
            neighbors = np.argsort(dist)[1:4].tolist()
            self.client.send_message("/node/edges", [int(row['roi_id'])] + neighbors)
            
            if i % 20 == 0: time.sleep(0.005) # Proteção de Buffer

    def run_query_loop(self):
        print("\n🧠 ORÁCULO ATIVO: AGUARDANDO VALOR X")
        while True:
            try:
                query = input("\n[CONSULTA] Digite o ID do ROI: ")
                if query.lower() == 'sair': break
                
                target_id = int(query)
                if target_id not in self.df['roi_id'].values:
                    print("❌ ROI não encontrado.")
                    continue

                # DISPARO DE INFERÊNCIA
                self.client.send_message("/brain/glow", [1.8])
                curr_id = target_id
                
                # Simula trajetória geodésica no Plexus
                for _ in range(12):
                    self.client.send_message("/inference/pulse", [int(curr_id)])
                    time.sleep(0.12)
                    # Busca o próximo vizinho por afinidade de severidade
                    curr_id = (curr_id + 7) % len(self.df) 
                
                self.client.send_message("/brain/glow", [1.0])
                print(f"✅ Trajetória do ROI {target_id} renderizada.")

            except ValueError: pass

if __name__ == "__main__":
    oracle = BrainOracleSender()
    oracle.compute_geometry()
    oracle.sync_atlas()
    oracle.run_query_loop()
