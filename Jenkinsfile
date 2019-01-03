def problem = load('jenkins/simulations/mergingDroplets.groovy')

def defaultOrder = problem.defaultOrder()
def defaultElements = problem.defaultElements()
def defaultSteps = problem.defaultSteps()
def defaultDelta = problem.defaultDelta()
def defaultMobilityFormula = problem.defaultMobilityFormula()
def defaultChemicalPotentialFormula = problem.defaultChemicalPotentialFormula()
def defaultInitialSurfaceSnippet = problem.defaultInitialSurfaceSnippet()

pipeline {

    agent { 
        node { 
            label 'node2'
        }
    }

    options {
        timeout time: 24, unit:'HOURS'
    }

    environment {
        GIT_URL = "${scm.userRemoteConfigs[0].url}"
        GIT_BRANCH = "cahn-hilliard"
        BUILD_NUMBER = "${env.BUILD_NUMBER}"
    }

    parameters {
        string(
            name: 'ORDER',
            defaultValue: defaultOrder
        )
        string(
            name: 'ELEMENTS',
            defaultValue: defaultElements
        )
        string(
            name: 'STEPS',
            defaultValue: defaultSteps
        )
        string(
            name: 'DELTA',
            defaultValue: defaultDelta
        )
        string(
            name: 'MOBILITY_FORMULA',
            description: 'Formulae for mobility. Variable is x, constants are theta and lambda. Do not use spaces.',
            defaultValue: defaultMobilityFormula
        )
        string(
            name: 'CHEMICAL_POTENTIAL_FORMULA',
            description: 'Formulae for chemical potential. Variable is x, constants are theta and lambda. Do not use spaces.',
            defaultValue: defaultChemicalPotentialFormula
        )
        text(
            name: 'INITIAL_SURFACE_SNIPPET',
            description: 'CPP code snippet which should return a value in x,y (x,y are double inputs)',
            defaultValue: defaultInitialSurfaceSnippet
        )
    }

    stages {

        stage('Checkout') {
            steps {
                echo "Checking out ${GIT_URL} on branch ${GIT_BRANCH}"
                git branch: "${GIT_BRANCH}", url: "${GIT_URL}"
            }
        }

        stage('Install dependencies') {
            steps {
                sh '''#!/bin/bash
                    source /etc/profile
                    source /usr/share/Modules/init/bash

                    module load gcc/7.2.0 cmake/3.11.1
                    module unload galois
                    module list

                    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$HOME/usr/local/lib
                    export CDIR=$(pwd)

                    cd $CDIR
                    wget http://iss.ices.utexas.edu/projects/galois/downloads/Galois-2.2.1.tar.gz
                    rm -rf Galois-2.2.1 || true
                    tar xzvf Galois-2.2.1.tar.gz
                    cd Galois-2.2.1/build
                    mkdir release
                    cd release
                    cmake -DBoost_INCLUDE_DIR=/opt/boost/include -DSKIP_COMPILE_APPS=ON ../..
                    make && \
                    make install DESTDIR=~/

                    wget https://dl.dropboxusercontent.com/s/ay1v02bijizv2dc/lb.zip?dl=0 -O lb.zip
                    unzip lb.zip \
                        && mv *.so $HOME/usr/local/lib/ \
                        && rm lb.zip
                '''
            }
        }

        stage('Build') {
            steps {
                sh '''#!/bin/bash
                    source /etc/profile
                    source /usr/share/Modules/init/bash

                    module load gcc/7.2.0 cmake/3.11.1
                    module unload galois
                    module list

                    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$HOME/usr/local/lib

                    echo "Local libraries"
                    LOCAL_LIB=$HOME/usr/local/lib
                    ls -la $LOCAL_LIB

                    echo "Local includes"
                    LOCAL_INC=$HOME/usr/local/include
                    ls -la $LOCAL_INC

                    echo "INITIAL_SURFACE_SNIPPET"

                    echo "Inserting INITIAL_SURFACE_SNIPPET snippet"
                    perl -pe 's/##INITIAL_SURFACE_SNIPPET##/`echo "$ENV{'INITIAL_SURFACE_SNIPPET'}"`/e' -i ./src/problems/cahn_hilliard/ch_2d.hpp
                    cat ./src/problems/cahn_hilliard/ch_2d.hpp


                    cmake . \
                    -DBLAS_LIBRARIES=$LOCAL_LIB/libblas.so \
                    -DLAPACK_LIBRARIES=$LOCAL_LIB/liblapack.so \
                    -DBOOST_INCLUDEDIR=/opt/boost/include \
                    -DGalois_INCLUDE_DIRS=$LOCAL_INC/Galois \
                    -DGalois_LIBRARIES=$LOCAL_LIB \
                    && make
                '''
            }
        }

        stage('Run') {
            steps {
                sh '''#!/bin/bash
                    ./cahn_hilliard \
                    ${ORDER} \
                    ${ELEMENTS} \
                    ${STEPS} \
                    ${DELTA} \
                    ${MOBILITY_FORMULA} \
                    ${CHEMICAL_POTENTIAL_FORMULA}
                '''
                stash name: 'results', includes: 'OUT/*.data,movie'
            }
        }

        stage('Process results') {
            steps {
                sh '''#!/bin/bash
                    cd OUT
                    gnuplot plot
                    ./movie
                    echo "Compressing results\n"
                    zip data.zip *.data
                    zip images.zip *.png
                    zip movies.zip *.mp4

                    RESULTS_DIR=/home/proj/jenkins_pub/pub/sim-$BUILD_NUMBER/

                    mkdir $RESULTS_DIR
                    cp images.zip $RESULTS_DIR/
                    cp movies.zip $RESULTS_DIR/
                    cp data.zip $RESULTS_DIR/
                    chmod -R 777 $RESULTS_DIR
                '''
            }
        }

    }

    post {
        always {
            cleanWs()
        }
    }
    
}
