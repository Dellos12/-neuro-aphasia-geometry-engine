
import pandas as pd
import os

def build_medical_knowledge():
    print("📚 Gerando Ondas de Referência (Literatura Médica)...")
    
    # Cada estado é uma 'Frequência de Fase' [Ricci, Química, Biofísica]
    data = [
        {
            "label": "HARMONIA_NOMINAL",
            "ricci_ref": 0.15, "chem_ref": 0.85, "elec_ref": 0.95,
            "descricao": "RESSONÂNCIA POSITIVA: O campo mantém a geodésica de saúde."
        },
        {
            "label": "DISSONANCIA_AFASICA",
            "ricci_ref": -0.55, "chem_ref": 0.20, "elec_ref": 0.30,
            "descricao": "INTERFERÊNCIA DESTRUTIVA: Colapso de fase detectado na propagação entre ROIs."
        }
    ]
    
    os.makedirs('data/gold', exist_ok=True)
    pd.DataFrame(data).to_parquet('data/gold/medical_literature.parquet')
    print("✅ Literatura Médica estacionada como Onda Guia.")

if __name__ == "__main__":
    build_medical_knowledge()
