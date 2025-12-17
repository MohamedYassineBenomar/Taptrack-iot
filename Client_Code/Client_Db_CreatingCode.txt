-- =========================================
-- 1. CREATE DATABASE
-- =========================================

-- Create the database (only if it does not exist)
CREATE DATABASE IF NOT EXISTS control_assistencia;

-- Use this database
USE control_assistencia;


-- =========================================
-- 2. CREATE TABLES
-- =========================================

-- Table: USUARI
-- Stores users (students, teachers, staff) and their card UID
CREATE TABLE usuari (
    id INT AUTO_INCREMENT PRIMARY KEY,
    dni VARCHAR(20) UNIQUE NOT NULL,
    nom_complet VARCHAR(100) NOT NULL,
    correu VARCHAR(100),
    telefon VARCHAR(20),
    targeta VARCHAR(50),  -- card UID (links card to this user)
    rol ENUM('professor', 'alumne', 'personal_servei')
);

-- Table: ASSIGNATURA
-- Stores subjects/courses
CREATE TABLE assignatura (
    id INT PRIMARY KEY,
    nom VARCHAR(100) NOT NULL,
    hores_totals INT
);

-- Table: UBICACIO
-- Stores location (classroom, floor, etc.)
CREATE TABLE ubicacio (
    id INT PRIMARY KEY,
    classe VARCHAR(50),
    planta VARCHAR(20)
);

-- Table: DISPOSITIU
-- Stores devices (card readers) and links each to a location
CREATE TABLE dispositiu (
    id INT AUTO_INCREMENT PRIMARY KEY,
    id_ubicacio INT UNIQUE,
    FOREIGN KEY (id_ubicacio) REFERENCES ubicacio(id)
);

-- Table: ASSISTIR
-- (For future use) stores attendance per user, subject and location
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

-- Table: FIXATGE
-- Stores each clock-in event: who, which device, date, time
CREATE TABLE fixatge (
    id_usuari INT NOT NULL,
    id_dispositiu INT NOT NULL,
    data DATE NOT NULL,
    hora TIME NOT NULL,

    FOREIGN KEY (id_usuari) REFERENCES usuari(id),
    FOREIGN KEY (id_dispositiu) REFERENCES dispositiu(id)
);


-- =========================================
-- 3. CREATE DB USER FOR PYTHON CLIENT
-- =========================================

-- Create a MySQL/MariaDB user that the Python app will use
-- (run this as root)
CREATE USER 'assist_admin'@'localhost' IDENTIFIED BY 'Assistencia123!';

-- Give this user full rights only on this database
GRANT ALL PRIVILEGES ON control_assistencia.* TO 'assist_admin'@'localhost';

-- Apply changes
FLUSH PRIVILEGES;


-- =========================================
-- 4. SAMPLE DATA (USER + LOCATION + DEVICE)
-- =========================================

-- Insert one user with a known card UID
-- Replace F4D44F06 with the real UID if needed
INSERT INTO usuari (dni, nom_complet, correu, telefon, targeta, rol)
VALUES (
    '12345678A',
    'Usuari Prova',
    'prova@example.com',
    '600000000',
    'F4D44F06',   -- card UID
    'alumne'
);

-- Insert one location (classroom)
INSERT INTO ubicacio (id, classe, planta)
VALUES (1, 'Aula 101', 'Planta 1');

-- Insert one device linked to that location
-- This will create dispositiu.id = 1 (used by the Python client)
INSERT INTO dispositiu (id_ubicacio) VALUES (1);

