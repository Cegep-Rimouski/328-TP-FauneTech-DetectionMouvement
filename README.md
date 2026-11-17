# DetectionMouvement

Projet de détection de mouvement par flux vidéo webcam en C++ avec OpenCV, prêt à l'emploi pour les étudiants sous **CLion** (compatible Windows 11 et Ubuntu, sans droits administrateur).

---

## 🚀 Première compilation & Portabilité

Ce projet s'appuie sur le mécanisme `FetchContent` de CMake pour télécharger et compiler localement les modules essentiels d'OpenCV 4.x de façon autonome et statique.

> [!NOTE]
> **Durée de la première compilation :**  
> Lors du tout premier lancement ou rechargement CMake, CLion téléchargera le code source d'OpenCV et effectuera sa compilation locale. Cette étape initiale prend généralement **quelques minutes** selon votre machine.  
> 
> **Une fois cette compilation terminée :**  
> - Les compilations suivantes seront quasi instantanées (seul votre code `main.cpp` sera recompilé).
> - Le projet est **100 % portable** et autonome : aucune installation préalable d'OpenCV sur votre système d'exploitation n'est requise, et **aucun droit administrateur (sudo / admin)** n'est nécessaire.

---

## 🛠️ Configuration et exécution dans CLion

### 1. Ouvrir le projet
1. Lancez **CLion**.
2. Cliquez sur **File > Open...** (ou *Ouvrir*) et sélectionnez le dossier racine du projet.
3. Laissez CLion charger et configurer le projet CMake automatiquement.

### 2. Lancer le projet
1. En haut à droite de l'interface CLion, vérifiez que la cible d'exécution sélectionnée est bien **`DetectionMouvement`**.
2. Cliquez sur le bouton vert **Run** (ou faites `Shift + F10`).
3. La fenêtre vidéo *DetectionMouvement* s'ouvrira avec le flux de votre webcam.

---

## 🎮 Commandes en cours d'exécution

Lorsque la fenêtre de flux vidéo est active :
- **`q`** ou **`Échap`** : Quitter proprement l'application.
- **`r`** : Réinitialiser l'arrière-plan de référence avec l'image actuelle (utile en cas de changement de luminosité ou si la scène a changé).

---

## 📁 Structure du projet

```text
.
├── CMakeLists.txt      # Configuration CMake avec FetchContent (OpenCV minimal)
├── main.cpp            # Algorithme de détection de mouvement & enregistrement
├── captures/           # Dossier de sauvegarde automatique des photos détectées (maximum 20 conservées)
│   └── .gitkeep
├── models/             # Dossier réservé pour les futurs fichiers de modèle
│   └── .gitkeep
└── README.md           # Documentation d'accueil et d'exécution
```
