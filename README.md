# 📌 Git Cheat Sheet - Comandos básicos + manejo de carpetas + repos compartidos

## 🔹 Configuración inicial (solo una vez)
git config --global user.name "Tu Nombre"
git config --global user.email "tuemail@example.com"

## 🔹 Crear o clonar un repositorio
git init                               # Inicia un repositorio en la carpeta actual
git clone URL_DEL_REPO                 # Clona un repositorio existente desde GitHub

## 🔹 Acceder a tu repositorio (cuando abres Git Bash nuevo)
cd /ruta/a/tu/repositorio              # Entra a la carpeta del repo
ls                                     # Lista los archivos dentro de la carpeta
pwd                                    # Muestra la ruta actual

## 🔹 Manejo de archivos/carpetas en terminal
ls                                     # Ver archivos/carpetas
ls -l                                  # Ver con detalles
cd nombre_carpeta                      # Entrar a una carpeta
cd ..                                  # Subir un nivel
mkdir nueva_carpeta                    # Crear carpeta
rm nombre_archivo                      # Borrar archivo
rm -r nombre_carpeta                   # Borrar carpeta y su contenido
clear                                  # Limpiar la pantalla de la terminal

## 🔹 Conectar repositorio local con GitHub
git remote add origin URL_DEL_REPO     # Conectar carpeta local al repositorio remoto

## 🔹 Flujo de trabajo típico
git status                             # Ver estado de los archivos
git add .                              # Añadir todos los cambios (nuevos y modificados)
git add nombre_archivo                 # Añadir un archivo específico
git commit -m "Mensaje del commit"     # Confirmar los cambios
git push origin main                   # Subir cambios a GitHub (rama main)

## 🔹 Traer cambios desde GitHub
git pull origin main                   # Descargar y fusionar cambios de GitHub

## 🔹 Eliminar archivos
git rm nombre_archivo                  # Eliminar archivo local y del repo
git rm --cached nombre_archivo         # Quitar archivo del repo pero mantenerlo localmente
# Si borraste manualmente un archivo:
git add nombre_archivo                 # Registrar el borrado
git commit -m "Elimino archivo nombre_archivo"
git push origin main

## 🔹 Atajos útiles
git add -u                             # Registrar modificaciones y eliminaciones (no archivos nuevos)
git log                                # Ver historial de commits
git diff                               # Ver diferencias antes de hacer commit

------------------------------------------------------------
# 📌 Repositorios compartidos (cuando alguien te pasa un repo)

## 🔹 Clonar un repositorio compartido
git clone https://github.com/usuario/proyecto.git
cd proyecto

## 🔹 Si tenés permiso de escritura (colaborador)
git add .
git commit -m "Mis cambios"
git push origin main

## 🔹 Si NO tenés permiso (ej: repo público)
1. Hacer un *Fork* en GitHub (lo copias a tu cuenta).
2. Clonar tu fork:
   git clone https://github.com/tuusuario/proyecto.git
   cd proyecto
3. Trabajar en tu copia y hacer *Pull Request* al original.

## 🔹 Agregar el repo original como remoto extra
git remote -v
git remote add upstream https://github.com/usuario/proyecto.git

## 🔹 Actualizar con cambios del repo original
git fetch upstream
git merge upstream/main
