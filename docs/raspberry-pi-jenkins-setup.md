# Raspberry Pi 3 + OpenWrt Jenkins Server Setup

## Overview

Your Raspberry Pi 3 (1GB RAM) is perfect for a lightweight Jenkins server!

**Prerequisites:**

- ✅ Raspberry Pi 3 (or 3 B+)
- ✅ OpenWrt installed
- ✅ Internet connection via OpenWrt
- ✅ SSH access to OpenWrt

---

## Option 1: Install Jenkins on OpenWrt

### Step 1: Enable USB Storage (Recommended)

OpenWrt needs more storage for Jenkins. Use a USB flash drive:

```bash
# Install USB support
opkg update
opkg install kmod-usb-storage kmod-fs-ext4 block-mount

# Format USB (on your laptop)
# Plug USB to laptop, format as ext4, label "jenkins"

# Mount USB on Pi
mount /dev/sda1 /mnt/usb
```

### Step 2: Install Java (Required)

```bash
opkg update
opkg install openjdk11-jre-headless
```

### Step 3: Install Jenkins

```bash
# Download Jenkins
cd /mnt/usb
wget https://get.jenkins.io/war-stable/latest/jenkins.war

# Create startup script
cat > /mnt/usb/jenkins.sh << 'EOF'
#!/bin/sh
cd /mnt/usb
java -jar jenkins.war --httpPort=8080
EOF
chmod +x /mnt/usb/jenkins.sh
```

### Step 4: Auto-start with Systemd or Init Script

Create OpenWrt init script `/etc/init.d/jenkins`:

```bash
#!/bin/sh /etc/rc.common

START=99
STOP=15

start() {
    cd /mnt/usb
    java -jar jenkins.war --httpPort=8080 > /var/log/jenkins.log 2>&1 &
}

stop() {
    killall java
}

restart() {
    stop
    start
}
```

Enable:

```bash
chmod +x /etc/init.d/jenkins
/etc/init.d/jenkins enable
/etc/init.d/jenkins start
```

---

## Option 2: Docker on OpenWrt (Recommended)

### Step 1: Install Docker on OpenWrt

```bash
# Add Docker repo
opkg update
opkg install docker docker-compose

# Start Docker
/etc/init.d/docker start
/etc/init.d docker enable
```

### Step 2: Run Jenkins in Docker

```bash
# Create volume for Jenkins data
docker volume create jenkins_home

# Run Jenkins
docker run -d \
  --name jenkins \
  -p 8080:8080 \
  -v jenkins_home:/var/jenkins_home \
  jenkinsci/blueocean

# Get initial password
docker exec jenkins cat /var/jenkins_home/secrets/initialAdminPassword
```

---

## Option 3: Lightweight Alternative - Jenkins with Alpine Linux

If OpenWrt is too limited, create a bootable Alpine Linux SD card:

### Step 1: Flash Alpine to SD Card

```bash
# Download Alpine for Raspberry Pi
wget https://alpine.linux.fau.edu/armhf/releases/raspberrypi/Alpine-rpi-3.18.0-armhf.tar.gz

# Extract to SD card root
```

### Step 2: Install Jenkins on Alpine

```bash
apk add openjdk11-jre-headless
wget https://get.jenkins.io/war-stable/latest/jenkins.war
java -jar jenkins.war
```

---

## Access Jenkins

1. **Find Pi's IP address:**

```bash
uci get network.lan.ipaddr
```

2. **Open browser:**

```
http://<PI-IP>:8080
```

3. **Unlock Jenkins:**

```bash
# Get password
docker exec jenkins cat /var/jenkins_home/secrets/initialAdminPassword
# Or
cat /mnt/usb/jenkins_home/secrets/initialAdminPassword
```

---

## Install Blue Ocean Plugin

1. Go to **Manage Jenkins** > **Manage Plugins**
2. Click **Available** tab
3. Search for **Blue Ocean**
4. Install and restart

---

## Build Configuration for Your Project

### 1. Create New Pipeline Job

- New Item > Pipeline
- Select "Pipeline script from SCM"
- Git > enter your repo URL

### 2. Configure Git Credentials

Manage Jenkins > Manage Credentials > Add SSH key

### 3. Run Build

The Jenkinsfile we created will automatically:

1. Checkout code
2. Build HOST platform
3. Run tests
4. (Optional) Build EMBEDDED

---

## Performance Tips for Pi 3

### Limit JVM Memory

```bash
# Add to java command
java -Xmx512m -jar jenkins.war
```

### Disable Unnecessary Plugins

Only install:

- Blue Ocean
- Pipeline
- Git

### Schedule Builds

Use quiet period to batch commits

---

## Troubleshooting

### "No space left on device"

```bash
# Use USB storage
# Or add swap
dd if=/dev/zero of=/swap bs=1M count=1024
mkswap /swap
swapon /swap
```

### "Java not found"

```bash
# Add Java to PATH
export PATH=$PATH:/usr/lib/jvm/java-11-openjdk/bin
```

### Slow builds

- Increase JVM heap: `-Xmx1g`
- Use SSD instead of SD card for Docker

---

## Network Setup (OpenWrt)

### Port Forwarding (if needed)

Network > Firewall > Port Forwards:

```
Protocol: TCP
Source: Anywhere
Port: 8080
Destination: <Pi-IP>:8080
```

---

## Summary

| Method            | Difficulty | Storage    | Performance |
| ----------------- | ---------- | ---------- | ----------- |
| Docker on OpenWrt | ⭐⭐       | USB needed | ⭐⭐⭐      |
| Native on OpenWrt | ⭐⭐⭐     | USB needed | ⭐⭐        |
| Alpine Linux      | ⭐⭐       | SD card    | ⭐⭐⭐⭐    |

**Recommendation:** Use **Option 2 (Docker)** for easiest management and updates.
