SCRIPT_NAME    = ./antivirusd.sh
WATCH_DIR      = ./target_dir
QUARANTINE_DIR = ./quar_dir
SLEEP_SECS     = 10

.PHONY: help setup run clean

help:
	@echo "Available commands:"
	@echo "  make setup  - Create directories and set script permissions"
	@echo "  make run    - Start the script"
	@echo "  make clean  - Remove the temporary files"

setup:
	@echo "Setting up directories..."
	mkdir -p $(WATCH_DIR) # -p is used to create parent dir if they are not already available, and also prevents errors if the dir exists already
	mkdir -p $(QUARANTINE_DIR)
	chmod +x $(SCRIPT_NAME)

run: setup
	@echo "Starting the antivirus..."
	$(SCRIPT_NAME) $(WATCH_DIR) $(QUARANTINE_DIR) $(SLEEP_SECS) # starting the script with arguments

clean:
	@echo "Cleaning up temporary files..."
	rm -f directory-info.last directory-info.new mal_files
