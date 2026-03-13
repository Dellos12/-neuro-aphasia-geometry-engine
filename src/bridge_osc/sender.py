
import pandas as pd
import numpy as np
import umap
from pythonosc import udp_client
import time

class BrainOracle:
    def __init__(self):
        self.client = udp_client.SimpleUDPClient("127.0.0.1", 5005)
        self.df = pd.read_parquet('data/gold/final_diagnosis.parquet')
        self.literature = pd.read_parquet('data/gold/medical_literature.parquet')
        self.embedding = None

    def compute_umap(self):
        print("🌀 Projetando Hiperplano UMAP 3D...")
        features = ['ricci_curv', 'drug_persistence', 'synaptic_efficiency', 'glutamate']
        reducer = umap.UMAP(n_neighbors=20, min_dist=0.1, n_components=3, metric='cosine')
        self.embedding = reducer.fit_transform(self.df[features].values)

    def sync_atlas(self):
        print("📡 Sincronizando Telemetria com o openFrameworks...")
        cause_map = {"ESTAVEL": 0, "FALHA_QUIMICA": 1, "FALHA_ELETRICA": 2}

        for i, row in self.df.iterrows():
            x, y, z = self.embedding[i]
            cause_id = cause_map.get(row['root_cause'], 0)
            severity = float(row['aphasia_severity'])

            # /roi/setup: [id, x, y, z, cause, severity]
            self.client.send_message("/roi/setup", [
                int(row['roi_id']), float(x), float(y), float(z), 
                int(cause_id), severity
            ])
            
            # /node/edges: [id, neighbor_ids...]
            dist = np.linalg.norm(self.embedding - self.embedding[i], axis=1)
            neighbors = np.argsort(dist)[1:4].tolist()
            self.client.send_message("/node/edges", [int(row['roi_id'])] + neighbors)
            
            if i % 20 == 0: time.sleep(0.005)

    def query(self, roi_id):
        # Lógica de Alinhamento de Fase (Cosseno)
        roi_row = self.df[self.df['roi_id'] == roi_id].iloc[0]
        v_roi = np.array([roi_row['ricci_curv'], roi_row['drug_persistence'], roi_row['synaptic_efficiency']])
        
        best_score = -1
        best_msg = ""
        
        for _, lit in self.literature.iterrows():
            v_lit = np.array([lit['ricci_ref'], lit['chem_ref'], lit['elec_ref']])
            score = np.dot(v_roi, v_lit) / (np.linalg.norm(v_roi) * np.linalg.norm(v_lit) + 1e-9)
            if score > best_score:
                best_score = score
                best_msg = lit['descricao']
        
        return best_msg, best_score

    def loop(self):
        print("\n🧠 ORÁCULO ATIVO: AGUARDANDO ID")
        while True:
            try:
                user_in = input("\n[CONSULTA] ID: ")
                if user_in == 'sair': break
                roi_id = int(user_in)
                
                msg, score = self.query(roi_id)
                print(f"📢 ORÁCULO ({score:.2%}):\n> {msg}")
                
                # Dispara o pulso visual (A "Onda" no C++)
                self.client.send_message("/inference/pulse", [roi_id])
                self.client.send_message("/brain/glow", [1.8])
                time.sleep(1)
                self.client.send_message("/brain/glow", [1.0])
            except: pass

if __name__ == "__main__":
    oracle = BrainOracle()
    oracle.compute_umap()
    oracle.sync_atlas()
    oracle.loop()
