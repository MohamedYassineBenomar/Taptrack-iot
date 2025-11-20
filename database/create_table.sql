CREATE DATABASE IF NOT EXISTS control_assistencia;
USE control_assistencia;

----------------------------------------------------------
--                   TAULA: USUARI
----------------------------------------------------------
CREATE TABLE usuari (
    id INT AUTO_INCREMENT PRIMARY KEY,
    dni VARCHAR(20) UNIQUE NOT NULL,
    nom_complet VARCHAR(100) NOT NULL,
    correu VARCHAR(100),
    telefon VARCHAR(20),
    targeta VARCHAR(50),
    rol ENUM('professor', 'alumne', 'personal_servei')
);

----------------------------------------------------------
--                   TAULA: ASSIGNATURA
----------------------------------------------------------
CREATE TABLE assignatura (
    id INT PRIMARY KEY,
    nom VARCHAR(100) NOT NULL,
    hores_totals INT
);

----------------------------------------------------------
--                   TAULA: UBICACIO
----------------------------------------------------------
CREATE TABLE ubicacio (
    id INT PRIMARY KEY,
    classe VARCHAR(50),
    planta VARCHAR(20)
);

----------------------------------------------------------
--                   TAULA: DISPOSITIU
----------------------------------------------------------
CREATE TABLE dispositiu (
    id INT AUTO_INCREMENT PRIMARY KEY,
    id_ubicacio INT UNIQUE,
    FOREIGN KEY (id_ubicacio) REFERENCES ubicacio(id)
);

----------------------------------------------------------
--      RELACIÓ: ASSISTIR (USUARI ↔ ASSIGNATURA ↔ UBICACIÓ)
----------------------------------------------------------
CREATE TABLE assistir (
    id_usuari INT NOT NULL,
    id_assignatura INT NOT NULL,
    id_ubicacio INT NOT NULL,
    data DATE NOT NULL,
    hora TIME NOT NULL,
    estat ENUM('present', 'absent', 'retard','falta_justificada'),

    FOREIGN KEY (id_usuari) REFERENCES usuari(id),
    FOREIGN KEY (id_assignatura) REFERENCES assignatura(id),
    FOREIGN KEY (id_ubicacio) REFERENCES ubicacio(id)
);

----------------------------------------------------------
--      RELACIÓ: FIXATGE (USUARI ↔ DISPOSITIU)
----------------------------------------------------------
CREATE TABLE fixatge (
    id_usuari INT NOT NULL,
    id_dispositiu INT NOT NULL,
    data DATE NOT NULL,
    hora TIME NOT NULL,

    FOREIGN KEY (id_usuari) REFERENCES usuari(id),
    FOREIGN KEY (id_dispositiu) REFERENCES dispositiu(id)
);
