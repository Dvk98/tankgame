source ~/repo/emsdk/emsdk_env.sh && \
rm -rf build_client && \
mkdir build_client && \
cd build_client && \
emconfigure cmake -DCMAKE_BUILD_TYPE=Debug .. && \
emmake make -j4 client && \
cp ../index.html index.html && \ 
python3 -m http.server