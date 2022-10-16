cp LabJack_T7_1.substitutions LabJack_T7_2.substitutions
cp LabJack_T7_1.substitutions LabJack_T7_3.substitutions
sed -i s/:1:/:2:/ LabJack_T7_2.substitutions
sed -i s/_1_/_2_/ LabJack_T7_2.substitutions
sed -i s/:1:/:3:/ LabJack_T7_3.substitutions
sed -i s/_1_/_3_/ LabJack_T7_3.substitutions
