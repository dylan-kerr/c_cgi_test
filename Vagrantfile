Vagrant.configure("2") do |config|
  config.vm.box = "ubuntu/artful64"

  # Increase RAM and CPU allocation.
  config.vm.provider "virtualbox" do |v|
    v.memory = 2048
    v.cpus = 2
    v.customize ["modifyvm", :id, "--uartmode1", "disconnected"]
  end

  # Expose ports between the host and VM.
  config.vm.network "forwarded_port", guest: 5000, host: 5000

  # config.vm.synced_folder "../data", "/vagrant_data"

  # Prevent "stdin: is not a tty" errors in Ubuntu as per https://github.com/hashicorp/vagrant/issues/1673.
  config.vm.provision "fix-no-tty", type: "shell" do |s|
    s.privileged = false
    s.inline = "sudo sed -i '/tty/!s/mesg n/tty -s \\&\\& mesg n/' /root/.profile"
  end

  config.vm.provision "docker"

  # Install Docker Compose and its bash completion scripts.
  config.vm.provision "shell", inline: <<-SHELL
    curl -sSL https://github.com/docker/compose/releases/download/1.19.0/docker-compose-`uname -s`-`uname -m` > /usr/local/bin/docker-compose
    chmod +x /usr/local/bin/docker-compose
    curl -sSL https://raw.githubusercontent.com/docker/compose/$(docker-compose version --short)/contrib/completion/bash/docker-compose -o /etc/bash_completion.d/docker-compose

    apt install -y build-essential gdb valgrind
  SHELL

end
