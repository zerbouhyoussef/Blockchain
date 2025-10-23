# 🧱 Atelier 1 – Implémentation d’une Blockchain en C++

## 🎓 Contexte
Projet réalisé dans le cadre du module *Blockchain* du Master IASD (Intelligence Artificielle et Science des Données), FST Tanger – Année universitaire 2025/2026.  
Encadré par **Pr. Ikram BENABDELOUAHAB**.

## 📦 Contenu du dépôt
| Fichier | Description |
|----------|--------------|
| `exercice1.cpp` | Implémentation de l’arbre de Merkle |
| `exercice2.cpp` | Proof of Work (PoW) avec mesure du temps de minage |
| `exercice3.cpp` | Proof of Stake (PoS) avec sélection de validateur |
| `exercice4.cpp` | Mini blockchain intégrant PoW et PoS |

## 🧠 Objectifs pédagogiques
- Comprendre la structure et le fonctionnement d’une blockchain.
- Manipuler les algorithmes de hachage (SHA256).
- Implémenter PoW et PoS en C++.
- Comparer expérimentalement les performances des deux approches.

## ⚙️ Compilation
Assurez-vous d’avoir OpenSSL :
```bash
sudo apt install libssl-dev
for i in {1..4}; do g++ exercice$i.cpp -o exercice$i -lcrypto -lssl; done
