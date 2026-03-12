# Jenkins on Raspberry Pi 3B - Local AI Agent Guide

This document provides instructions for an AI agent running directly on Raspberry Pi 3B to install and configure Jenkins.

---

## Environment Context

- **AI Location:** Running directly on Raspberry Pi 3B
- **OS:** Linux (likely Raspberry Pi OS or similar)
- **Architecture:** ARM64 (aarch64)

---

## Step 1: System Check

```bash
# Check OS
cat /etc/os-release

# Check memory
free -h

# Check disk space
df -h

# Check architecture
uname -m
# Expected: aarch64 for Pi 3/4, armv7l for Pi 2

# Check if Docker is installed
which docker
docker --version
```

---

## Step 2: Install Docker (if not present)

### Install Docker

```bash
# Install Docker if not present
curl -fsSL https://get.docker.com | sh

# Add current user to docker group
sudo usermod -aG docker $USER

# Enable and start Docker
sudo systemctl enable docker
sudo systemctl start docker

# Verify
docker --version
docker ps
```

### Configure Docker

```bash
sudo mkdir -p /etc/docker
sudo tee /etc/docker/daemon.json <<EOF
{
  "log-driver": "json-file",
  "log-opts": {
    "max-size": "10m",
    "max-file": "3"
  },
  "storage-driver": "overlay2"
}
EOF

sudo systemctl restart docker
```

---

## Step 3: Install Jenkins

### Create Docker Network

```bash
docker network create jenkins
```

### Run Jenkins Container

```bash
docker run -d \
  --name jenkins \
  -p 8080:8080 \
  -p 50000:50000 \
  -v jenkins_home:/var/jenkins_home \
  -v /var/run/docker.sock:/var/run/docker.sock \
  --network jenkins \
  --restart unless-stopped \
  jenkinsci/blueocean
```

### Wait for Startup

```bash
# Wait for Jenkins to initialize (30-60 seconds)
sleep 30

# Check if it's running
docker ps | grep jenkins
```

---

## Step 4: Initial Setup

### Get Initial Admin Password

```bash
# Method 1: From container logs
docker logs jenkins 2>&1 | grep -A 5 "Please use the following password"

# Method 2: Direct file
docker exec jenkins cat /var/jenkins_home/secrets/initialAdminPassword
```

### Unlock via Browser

1. Open browser to `http://localhost:8080`
2. Enter the admin password

### Install Plugins

```bash
# Automated plugin installation via Jenkins CLI
# After unlocking, plugins will be installed automatically
# For manual installation:

# Download jenkins-cli.jar
wget http://localhost:8080/jnlpJars/jenkins-cli.jar

# Install plugins (examples)
java -jar jenkins-cli.jar -s http://localhost:8080 install-plugin git github blueocean
```

---

## Step 5: Create Admin User

This step typically requires browser interaction. For CLI automation:

```bash
# Create admin user via Jenkins CLI
java -jar jenkins-cli.jar -s http://localhost:8080 create-user admin password "Admin User" admin@example.com
```

Or skip by using the initial admin password for all operations.

---

## Step 6: GitHub Integration

### Create GitHub Personal Access Token

1. Go to GitHub > Settings > Developer settings > Personal access tokens
2. Generate new token with `repo` and `workflow` scopes
3. Copy the token

### Add Credentials to Jenkins

```bash
# Add GitHub credentials via Jenkins CLI
# This requires the script console or GUI
# Use the Jenkins UI:
# Manage Jenkins > Manage Credentials > Add Credentials
# Kind: Username with password
# Username: your-github-username
# Password: your-github-token
```

### Configure GitHub Webhook

```bash
# No direct CLI - use GitHub UI
# Go to: Repo Settings > Webhooks > Add webhook
# Payload URL: http://<PI-IP>:8080/github-webhook/
```

---

## Step 7: Create First Pipeline

### Create Job Directory

```bash
mkdir -p /home/pi/jenkins-jobs
cd /home/pi/jenkins-jobs
```

### Create Pipeline Job

```bash
# Create job configuration XML
cat > my-pipeline.xml <<'EOF'
<project>
  <description>Embedded HSM Build Pipeline</description>
  <keepDependencies>false</keepDependencies>
  <properties/>
  <scm class="hudson.plugins.git.GitSCM">
    <configVersion>2</configVersion>
    <userRemoteConfigs>
      <hudson.plugins.git.UserRemoteConfig>
        <url>https://github.com/datdang-dev/embedded-firmware-lib.git</url>
      </hudson.plugins.git.UserRemoteConfig>
    </userRemoteConfigs>
    <branches>
      <hudson.plugins.git.BranchSpec>
        <name>*/main</name>
      </hudson.plugins.git.BranchSpec>
    </branches>
  </scm>
  <canRoam>true</canRoam>
  <disabled>false</disabled>
  <blockBuildWhenDownstreamBuilding>false</blockBuildWhenDownstreamBuilding>
  <blockBuildWhenUpstreamBuilding>false</blockBuildWhenUpstreamBuilding>
  <triggers>
    <hudson.triggers.SCMTrigger>
      <spec>H/5 * * * *</spec>
      <ignorePostCommitHooks>false</ignorePostCommitHooks>
    </hudson.triggers.SCMTrigger>
  </triggers>
  <concurrentBuild>false</concurrentBuild>
  <builders>
    <org.jenkinsci.plugins.workflow.cps.CpsScmFlowDefinition>
      <scriptPath>Jenkinsfile</scriptPath>
      <lightweight>true</lightweight>
    </org.jenkinsci.plugins.workflow.cps.CpsScmFlowDefinition>
  </builders>
  <publishers/>
  <buildWrappers/>
</project>
EOF

# Create the job
curl -s -X POST http://localhost:8080/createItem?name=embedded-hsm \
  -H "Content-Type: application/xml" \
  -u admin:YOUR_API_TOKEN \
  --data-binary @my-pipeline.xml
```

---

## Daily Operations

### Check Status

```bash
# Docker status
docker ps | grep jenkins

# Jenkins health
curl -s http://localhost:8080/api/json | head -c 200

# Check logs
docker logs --tail 50 jenkins
```

### Trigger Build

```bash
# Trigger build
curl -s -X POST http://localhost:8080/job/embedded-hsm/build

# Wait and check result
sleep 10
curl -s http://localhost:8080/job/embedded-hsm/lastBuild/api/json | grep result
```

### View Logs

```bash
# Get build console output
curl -s http://localhost:8080/job/embedded-hsm/lastBuild/consoleText

# Or full logs
docker logs jenkins
```

---

## Troubleshooting

### Container Won't Start

```bash
# Check logs
docker logs jenkins

# Check disk space
df -h

# Check memory
free -m

# If out of memory, recreate with limit:
docker rm jenkins
docker run -d --name jenkins -p 8080:8080 -p 50000:50000 -m 512m -v jenkins_home:/var/jenkins_home jenkinsci/blueocean
```

### Port Already in Use

```bash
# Check what's using port 8080
sudo netstat -tlnp | grep 8080

# Change Jenkins port:
docker rm jenkins
docker run -d --name jenkins -p 8090:8080 -v jenkins_home:/var/jenkins_home jenkinsci/blueocean
```

### Plugin Installation Fails

```bash
# Restart Jenkins and try again
docker restart jenkins
sleep 30

# Manual plugin install
java -jar jenkins-cli.jar -s http://localhost:8080 install-plugin pipeline-stage-view
```

---

## Build CMake Project in Pipeline

### Required Tools in Container

```bash
# Install build tools in container
docker exec -it jenkins apt-get update
docker exec -it jenkins apt-get install -y cmake build-essential git

# Or build image with tools pre-installed
docker build -t jenkins-with-cmake - <<'EOF'
FROM jenkinsci/blueocean
USER root
RUN apt-get update && apt-get install -y cmake build-essential git
USER jenkins
EOF
```

### Example Jenkinsfile

```groovy
pipeline {
    agent {
        docker {
            image 'jenkins-with-cmake'
            args '-v /var/run/docker.sock:/var/run/docker.sock'
        }
    }

    stages {
        stage('Checkout') {
            steps {
                checkout scm
            }
        }

        stage('Build HOST') {
            steps {
                sh 'cmake -DTARGET_PLATFORM=HOST -B build && cmake --build build'
            }
        }

        stage('Test') {
            steps {
                sh 'ctest --test-dir build --output-on-failure'
            }
        }

        stage('Archive') {
            steps {
                archiveArtifacts artifacts: 'build/**/*', fingerprint: true
            }
        }
    }

    post {
        always {
            cleanWs()
        }
    }
}
```

---

## Quick Reference

| Task           | Command                                                                                           |
| -------------- | ------------------------------------------------------------------------------------------------- |
| Install Docker | `curl -fsSL https://get.docker.com \| sh`                                                         |
| Start Jenkins  | `docker run -d --name jenkins -p 8080:8080 -v jenkins_home:/var/jenkins_home jenkinsci/blueocean` |
| Get password   | `docker exec jenkins cat /var/jenkins_home/secrets/initialAdminPassword`                          |
| Restart        | `docker restart jenkins`                                                                          |
| Stop           | `docker stop jenkins`                                                                             |
| View logs      | `docker logs --tail 100 jenkins`                                                                  |
| Trigger build  | `curl -s -X POST http://localhost:8080/job/JOB_NAME/build`                                        |
| Check status   | `curl -s http://localhost:8080/api/json`                                                          |
