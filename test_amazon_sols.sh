GRAPH_FOLDER="/home/graph_collection/vr_instances"

parallel --progress --bar --linebuffer --joblog exit_log.txt /home/jholten/KaMIS-ml/deploy/analyze mis $GRAPH_FOLDER/{}/{}.graph $GRAPH_FOLDER/{}/solution.txt amazon ::: $(ls $GRAPH_FOLDER) | tee console_log.txt 
