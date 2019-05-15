#!/bin/bash
# --- PREPARE LOGs ---
rm -f gource_log_*.txt combined_gource_log.txt

`gource --git-log-command` | sed 's/^\(user:\)\([^(]*\).*/\1\2/' > gource_log_gigamesh.txt
./gource_log_conv/gource_log_conv -f gource_log_gigamesh.txt -p GigaMesh/source/ > gource_log_gigamesh_custom.txt

cd ~/Documents/diss
`gource --git-log-command` | sed 's/^\(user:\)\([^(]*\).*/\1\2/' > ~/Desktop/GigaMesh/gource_log_diss.txt
cd  ~/Desktop/GigaMesh
./gource_log_conv/gource_log_conv -f gource_log_diss.txt -p GigaMesh/Dissertation/ > gource_log_diss_custom.txt

cd ~/Documents/CVA_Beiheft
`gource --git-log-command` | sed 's/^\(user:\)\([^(]*\).*/\1\2/' > ~/Desktop/GigaMesh/gource_log_cva_beiheft.txt
cd  ~/Desktop/GigaMesh
./gource_log_conv/gource_log_conv -f gource_log_cva_beiheft.txt -p GigaMesh/Publications/CVA_Beiheft_2012/ > gource_log_cva_beiheft_custom.txt

cd ~/Documents/papers/2010/vast2010_keilschrift
`gource --git-log-command` | sed 's/^\(user:\)\([^(]*\).*/\1\2/' > ~/Desktop/GigaMesh/gource_log_vast2010_keilschrift.txt
cd  ~/Desktop/GigaMesh
./gource_log_conv/gource_log_conv -f gource_log_vast2010_keilschrift.txt -p GigaMesh/Publications/vast2010_cuneiform/ > gource_log_vast2010_keilschrift_custom.txt

cd ~/Documents/papers/2010/vast2010_abrollungen
`gource --git-log-command` | sed 's/^\(user:\)\([^(]*\).*/\1\2/' > ~/Desktop/GigaMesh/gource_log_vast2010_abrollungen.txt
cd  ~/Desktop/GigaMesh
./gource_log_conv/gource_log_conv -f gource_log_vast2010_abrollungen.txt -p GigaMesh/Publications/vast2010_rollouts/ > gource_log_vast2010_abrollungen_custom.txt

cat gource_log_*_custom*.txt | sort -n > combined_gource_log.txt
# --- MAKE VIDEO ---
# -r FPS
# -i 0 = disable  hiding files
# -s = seconds per day
#gource gource_log_gigamesh.txt -1280x720 -r 30 -i 0 -s 0.12 -logo forms/GigaMesh_Logo_Inv_Small.png --logo-offset 1210x650 -o - | ffmpeg -y -b 3000K -r 30 -f image2pipe -vcodec ppm -i - -vcodec libx264 -vpre slow gource2.mp4
#gource combined_gource_log.txt --log-format custom -1280x720 -r 30 -i 0 -s 0.12 --highlight-users -logo forms/GigaMesh_Logo_Inv_Small.png --logo-offset 1210x650 -o - | ffmpeg -y -b 3000K -r 30 -f image2pipe -vcodec ppm -i - -vcodec libx264 -vpre slow gource4.mp4
#gource combined_gource_log.txt --log-format custom -1920x1080 -r 30 -i 0 -s 0.12 --highlight-users \
       --highlight-dirs --file-extensions\
       -logo forms/GigaMesh_Logo_Inv_Small.png --logo-offset 1850x1010 -o - | ffmpeg -y -b 6000K -r 30 -f image2pipe -vcodec ppm -i - -vcodec libx264 -vpre slow gource4.mp4
