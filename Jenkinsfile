pipeline {
    agent any

    environment {
        PLATFORM_HOST = 'HOST'
        PLATFORM_EMBEDDED = 'EMBEDDED'
    }

    stages {
        stage('Checkout') {
            steps {
                checkout scm
            }
        }

        stage('Build HOST') {
            steps {
                dir("${WORKSPACE}") {
                    sh '''
                        cmake -DTARGET_PLATFORM=HOST -B build
                        cmake --build build
                    '''
                }
            }
        }

        stage('Test') {
            steps {
                dir("${WORKSPACE}/build") {
                    sh 'ctest --output-on-failure'
                }
            }
        }

        stage('Static Analysis') {
            steps {
                sh '''
                    which clang-tidy && \
                    cd build && \
                    clang-tidy -quiet -format clang ../src/**/*.c 2>/dev/null || \
                    echo "clang-tidy not available, skipping"
                '''
            }
        }

        stage('Build EMBEDDED') {
            when {
                expression { fileExists('toolchain.cmake') }
            }
            steps {
                sh '''
                    cmake -DTARGET_PLATFORM=EMBEDDED \
                          -DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake \
                          -B build_embedded
                    cmake --build build_embedded
                '''
            }
        }

        stage('Code Coverage') {
            steps {
                sh '''
                    pip install gcovr --quiet || true
                    cmake -DTARGET_PLATFORM=HOST \
                          -DENABLE_COVERAGE=ON \
                          -B build_cov
                    cmake --build build_cov
                    cd build_cov && ctest --output-on-failure
                '''
            }
            post {
                always {
                    publishHTML(target: [
                        reportDir: 'build_cov',
                        reportFiles: 'coverage.html',
                        reportName: 'Coverage Report'
                    ])
                }
            }
        }
    }

    post {
        always {
            cleanWs()
        }
        success {
            echo 'Pipeline completed successfully!'
        }
        failure {
            echo 'Pipeline failed!'
        }
    }
}
