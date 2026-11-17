#include <algorithm>
#include <cctype>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <sstream>
#include <vector>

// Constantes de réglage de la détection
constexpr double SEUIL_SURFACE_MINIMALE = 3500.0; // Surface minimale en pixels pour déclencher une détection
constexpr int SEUIL_DIFFERENCE_BINAIRE = 30; // Écart de luminosité requis par pixel (0 à 255)
constexpr int LARGEUR_IMAGE = 640;
constexpr int HAUTEUR_IMAGE = 480;
constexpr size_t MAX_IMAGES_CONSERVEES = 20; // Nombre maximal d'images conservées dans le dossier de capture

// Constantes temporelles (anti-rebond & stabilité)
constexpr int FRAMES_CONFIRMATION_MIN = 3; // Nombre d'images consécutives avec détection requises (~100 ms)
constexpr double DELAI_COOLDOWN_SECONDES = 3.0; // Délai minimal entre deux prises de vue (en secondes)

/**
 * @brief Génère un chemin de fichier unique basé sur l'horodatage exact.
 */
std::string genererNomFichierCapture()
{
    auto maintenant = std::chrono::system_clock::now();
    auto tempsT = std::chrono::system_clock::to_time_t(maintenant);
    auto millisecondes = std::chrono::duration_cast<std::chrono::milliseconds>(
                             maintenant.time_since_epoch()) %
                         1000;

    std::stringstream ss;
    ss << "captures/capture_"
            << std::put_time(std::localtime(&tempsT), "%Y%m%d_%H%M%S_")
            << std::setfill('0') << std::setw(3) << millisecondes.count() << ".jpg";
    return ss.str();
}

/**
 * @brief Conserve au maximum un nombre défini d'images dans le dossier de
 * capture. Les images les plus anciennes sont supprimées en premier.
 *
 * @param dossier Chemin du dossier de captures.
 * @param maxImages Nombre maximal d'images à conserver.
 */
void limiterNombreCaptures(const std::filesystem::path& dossier,
                           size_t maxImages)
{
    std::error_code ec;
    if (!std::filesystem::exists(dossier, ec) ||
        !std::filesystem::is_directory(dossier, ec))
    {
        return;
    }

    std::vector<std::filesystem::directory_entry> fichiers;
    for (const auto& entree: std::filesystem::directory_iterator(dossier, ec))
    {
        if (entree.is_regular_file(ec))
        {
            std::string ext = entree.path().extension().string();
            for (char& c: ext)
            {
                c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            }
            if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".bmp")
            {
                fichiers.push_back(entree);
            }
        }
    }

    if (fichiers.size() <= maxImages)
    {
        return;
    }

    // Tri du plus ancien au plus récent (selon la date de modification, puis le
    // nom de fichier)
    std::sort(fichiers.begin(), fichiers.end(),
              [](const std::filesystem::directory_entry& a,
                 const std::filesystem::directory_entry& b)
              {
                  std::error_code ecA, ecB;
                  auto tA = a.last_write_time(ecA);
                  auto tB = b.last_write_time(ecB);
                  if (!ecA && !ecB && tA != tB)
                  {
                      return tA < tB;
                  }
                  return a.path().filename().string() <
                         b.path().filename().string();
              });

    size_t nombreASupprimer = fichiers.size() - maxImages;
    for (size_t i = 0; i < nombreASupprimer; ++i)
    {
        std::error_code ecSuppr;
        std::filesystem::remove(fichiers[i].path(), ecSuppr);
        if (!ecSuppr)
        {
            std::cout
                    << "[DetectionMouvement] Nettoyage : capture ancienne supprimee : "
                    << fichiers[i].path().filename().string()
                    << " (limite max : " << maxImages << " images)" << std::endl;
        }
    }
}

/**
 * @brief Point d'interception et enregistrement de l'image lors d'une détection
 * confirmée.
 *
 * Extrait la région d'intérêt avec une bordure de sécurité et l'enregistre sur
 * disque.
 *
 * @param image Image complète au moment de la détection.
 * @param zoneEnglobante Boîte délimitant la cible en mouvement.
 */
void traiterCaptureMouvement(const cv::Mat& image,
                             const cv::Rect& zoneEnglobante)
{
    // Calcul d'une marge de sécurité (15%) autour de la zone détectée
    int margeX = static_cast<int>(zoneEnglobante.width * 0.15);
    int margeY = static_cast<int>(zoneEnglobante.height * 0.15);

    int x = std::max(0, zoneEnglobante.x - margeX);
    int y = std::max(0, zoneEnglobante.y - margeY);
    int largeur = std::min(image.cols - x, zoneEnglobante.width + (2 * margeX));
    int hauteur = std::min(image.rows - y, zoneEnglobante.height + (2 * margeY));

    cv::Rect zoneAgrandie(x, y, largeur, hauteur);
    cv::Mat regionCapturee = image(zoneAgrandie);

    // Enregistrement de l'image de la cible
    std::string nomFichier = genererNomFichierCapture();
    bool succes = cv::imwrite(nomFichier, regionCapturee);

    if (succes)
    {
        std::filesystem::path cheminAbsolu = std::filesystem::absolute(nomFichier);
        std::cout << "\n>>> [DetectionMouvement] Image sauvegardee : "
                << cheminAbsolu.string() << " (Taille : " << largeur << "x"
                << hauteur << " px)" << std::endl;

        // Maintien d'un maximum de captures conservées
        limiterNombreCaptures("captures", MAX_IMAGES_CONSERVEES);
    }
    else
    {
        std::cerr << "Erreur : Impossible d'enregistrer le fichier " << nomFichier
                << std::endl;
    }
}

int main()
{
    // Création automatique du dossier de sauvegarde des captures et application
    // de la limite
    std::filesystem::create_directories("captures");
    limiterNombreCaptures("captures", MAX_IMAGES_CONSERVEES);

    std::cout << "[DetectionMouvement] Repertoire de travail : "
            << std::filesystem::current_path().string() << std::endl;

    // 1. Initialisation de la capture vidéo depuis la webcam
    cv::VideoCapture cap(0);
    if (!cap.isOpened())
    {
        std::cerr << "Erreur : Impossible d'acceder a la webcam (index 0)."
                << std::endl;
        return -1;
    }

    cap.set(cv::CAP_PROP_FRAME_WIDTH, LARGEUR_IMAGE);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, HAUTEUR_IMAGE);

    std::cout
            << "[DetectionMouvement] Stabilisation du capteur (reglage exposition)..."
            << std::endl;

    // Stabilisation initiale plus longue (~1 seconde) pour laisser
    // l'auto-exposition se caler
    cv::Mat frame;
    for (int i = 0; i < 35; ++i)
    {
        cap.read(frame);
    }

    if (frame.empty())
    {
        std::cerr << "Erreur : Flux video inaccessible ou vide." << std::endl;
        return -1;
    }

    // 2. Capture de l'arrière-plan de référence
    cv::Mat imageGrise;
    cv::Mat arrierePlan;
    cv::Mat imageFloutee;
    cv::Mat difference;
    cv::Mat masque;

    cv::cvtColor(frame, imageGrise, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(imageGrise, arrierePlan, cv::Size(21, 21), 0);

    std::cout << "[DetectionMouvement] Arriere-plan initialise." << std::endl;
    std::cout
            << "Commandes : [q] Quitter | [r] Reinitialiser le decor de reference"
            << std::endl;

    // Variables de gestion temporelle
    int compteurFramesStables = 0;
    auto tempsDerniereCapture =
            std::chrono::steady_clock::now() - std::chrono::seconds(10);
    auto tempsDernierAffichageAlerte =
            std::chrono::steady_clock::now() - std::chrono::seconds(10);

    // 3. Boucle principale de surveillance
    while (true)
    {
        if (!cap.read(frame) || frame.empty())
        {
            std::cerr << "Avertissement : Image vide recue du capteur." << std::endl;
            break;
        }

        // Image d'affichage pour les annotations
        cv::Mat affichage = frame.clone();

        // Conversion en niveaux de gris et lissage gaussien
        cv::cvtColor(frame, imageGrise, cv::COLOR_BGR2GRAY);
        cv::GaussianBlur(imageGrise, imageFloutee, cv::Size(21, 21), 0);

        // Soustraction d'arrière-plan
        cv::absdiff(arrierePlan, imageFloutee, difference);
        cv::threshold(difference, masque, SEUIL_DIFFERENCE_BINAIRE, 255,
                      cv::THRESH_BINARY);
        cv::dilate(masque, masque, cv::Mat(), cv::Point(-1, -1), 2);

        // Détection des contours
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(masque, contours, cv::RETR_EXTERNAL,
                         cv::CHAIN_APPROX_SIMPLE);

        bool mouvementDetecte = false;
        double surfaceMax = 0.0;
        cv::Rect zonePrincipale;

        for (const auto& contour: contours)
        {
            double surface = cv::contourArea(contour);

            if (surface >= SEUIL_SURFACE_MINIMALE)
            {
                mouvementDetecte = true;
                cv::Rect boite = cv::boundingRect(contour);

                // Tracé d'un rectangle vert (BGR : 0, 255, 0)
                cv::rectangle(affichage, boite, cv::Scalar(0, 255, 0), 2);

                if (surface > surfaceMax)
                {
                    surfaceMax = surface;
                    zonePrincipale = boite;
                }
            }
        }

        // Gestion de la stabilité temporelle (anti faux-positif de 1 frame)
        if (mouvementDetecte)
        {
            compteurFramesStables++;
        }
        else
        {
            compteurFramesStables = 0;
        }

        // Vérification de la période d'attente (cooldown)
        auto tempsActuel = std::chrono::steady_clock::now();
        std::chrono::duration<double> tempsEcoule =
                tempsActuel - tempsDerniereCapture;

        // Déclenchement de la capture photographique
        if (compteurFramesStables >= FRAMES_CONFIRMATION_MIN &&
            tempsEcoule.count() >= DELAI_COOLDOWN_SECONDES)
        {
            // Enregistrement sur disque
            traiterCaptureMouvement(frame, zonePrincipale);

            tempsDerniereCapture = tempsActuel;
            tempsDernierAffichageAlerte = tempsActuel;
            compteurFramesStables = 0;
        }

        // Indication visuelle de capture récente sur l'écran
        std::chrono::duration<double> alerteAffichee =
                tempsActuel - tempsDernierAffichageAlerte;
        if (alerteAffichee.count() < 1.2)
        {
            cv::putText(affichage, "PHOTO ENREGISTREE !", cv::Point(20, 40),
                        cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);
        }

        // Affichage des fenêtres vidéo et masque
        cv::imshow("DetectionMouvement", affichage);
        cv::imshow("Masque de detection", masque);

        // Gestion des commandes clavier
        char touche = static_cast<char>(cv::waitKey(30));
        if (touche == 'q' || touche == 'Q' || touche == 27)
        {
            std::cout << "[DetectionMouvement] Arret de la detection de mouvement."
                    << std::endl;
            break;
        }
        else if (touche == 'r' || touche == 'R')
        {
            imageFloutee.copyTo(arrierePlan);
            std::cout << "[DetectionMouvement] Arriere-plan recalibre avec succes."
                    << std::endl;
        }
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}
