# OpenMV model source notices
The retained FOMO, YOLO LC, YOLOv8, Face Landmarks and OpenMV BlazeFace artifacts are compiler-derived from OpenMV v5.0.1, commit 631681e5ac5ab332c920b6bf9cd3dba5cf299eac. Per-model source URLs and hashes are in ../MODEL_MANIFEST.json. No new license is assigned to their weights.
OpenMV example headers state MIT and retain their original copyright notices in the adapted examples. This does not independently establish a model-weights license. No separate model license was verified for the binary exports at the pinned commit; do not expand an example's license to the weights.
The Face Landmarks/BlazeFace family is attributed to Google MediaPipe. LICENSE.mediapipe preserves the MediaPipe project's Apache-2.0 text, without claiming independent proof of its application to every OpenMV quantized export.
OpenMV source licensing reference: https://github.com/openmv/openmv/blob/631681e5ac5ab332c920b6bf9cd3dba5cf299eac/README.md#licensing
The frozen ml postprocessors include their own upstream notices (some OpenMV modules have noncommercial licensing terms); consult the files under ThirdParty/openmv/scripts/libraries/ml for the original text.
