# +919080610799

# Send notification of usage information when the usage exceeds 50%, 90% and 100% of daily usage to the particular user.
# Send WhatsApp Alert. 

import pywhatkit
import time
import datetime
import logging
import os
import webbrowser
import pyautogui
import urllib.parse
from selenium import webdriver
from selenium.webdriver.chrome.options import Options
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
from webdriver_manager.chrome import ChromeDriverManager
from typing import Optional

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[logging.FileHandler("iot_whatsapp.log"), logging.StreamHandler()]
)
logger = logging.getLogger("IoT_WhatsApp")

class WhatsAppMessenger:
    """A class to handle WhatsApp messaging for IoT applications"""
    
    def __init__(self, wait_time: int = 30, close_tab: bool = True, use_selenium: bool = True):
        """
        Initialize the WhatsApp messenger
        
        Args:
            wait_time: Time to wait before closing the tab (seconds)
            close_tab: Whether to close the browser tab after sending
            use_selenium: Whether to use Selenium for more reliable automation
        """
        self.wait_time = wait_time
        self.close_tab = close_tab
        self.use_selenium = use_selenium
        self.driver = None
        logger.info("WhatsApp messenger initialized")
    
    def _initialize_selenium(self):
        """Initialize Selenium WebDriver for more reliable automation"""
        if self.driver is None and self.use_selenium:
            try:
                chrome_options = Options()
                chrome_options.add_argument("--start-maximized")
                # Uncomment the line below if you want to run headless
                # chrome_options.add_argument("--headless")
                
                self.driver = webdriver.Chrome(ChromeDriverManager().install(), options=chrome_options)
                logger.info("Selenium WebDriver initialized successfully")
                return True
            except Exception as e:
                logger.error(f"Failed to initialize Selenium WebDriver: {str(e)}")
                self.use_selenium = False
                return False
        return True
    
    def _close_selenium(self):
        """Close Selenium WebDriver if it's open"""
        if self.driver:
            try:
                self.driver.quit()
                self.driver = None
                logger.info("Selenium WebDriver closed")
            except Exception as e:
                logger.error(f"Error closing Selenium WebDriver: {str(e)}")
    
    def send_message(self, phone_number: str, message: str, delay: Optional[int] = None) -> bool:
        """
        Send a WhatsApp message to a specified phone number
        
        Args:
            phone_number: Phone number with country code (no + or spaces)
            message: Message to send
            delay: Optional delay in seconds before sending (default: None)
            
        Returns:
            bool: True if message sent successfully, False otherwise
        """
        # Ensure phone number format is correct (remove spaces, +, etc)
        phone_number = self._format_phone_number(phone_number)
        
        try:
            # Get current time for scheduling the message
            now = datetime.datetime.now()
            
            # Add a small delay to allow browser to open
            if delay:
                target_time = now + datetime.timedelta(seconds=delay)
                target_hour = target_time.hour
                target_minute = target_time.minute
            else:
                target_hour = now.hour
                target_minute = now.minute + 1  # Send a minute from now
            
            logger.info(f"Sending message to {phone_number} at {target_hour}:{target_minute}")
            
            # Send the message
            pywhatkit.sendwhatmsg(
                phone_no=phone_number,
                message=message,
                time_hour=target_hour,
                time_min=target_minute,
                wait_time=self.wait_time,
                tab_close=self.close_tab
            )
            
            logger.info(f"Message sent successfully to {phone_number}")
            return True
            
        except Exception as e:
            logger.error(f"Failed to send message to {phone_number}: {str(e)}")
            return False
    
    def send_message_instantly(self, phone_number: str, message: str) -> bool:
        """
        Send a WhatsApp message immediately without scheduling
        
        Args:
            phone_number: Phone number with country code (no + or spaces)
            message: Message to send
            
        Returns:
            bool: True if message sent successfully, False otherwise
        """
        # Ensure phone number format is correct
        phone_number = self._format_phone_number(phone_number)
        
        # Use custom selenium implementation if enabled
        if self.use_selenium:
            return self._send_with_selenium(phone_number, message)
        
        try:
            logger.info(f"Sending immediate message to {phone_number}")
            
            # Send message instantly with increased wait time
            pywhatkit.sendwhatmsg_instantly(
                phone_no=phone_number,
                message=message,
                wait_time=self.wait_time,
                tab_close=self.close_tab
            )
            
            logger.info(f"Immediate message sent successfully to {phone_number}")
            return True
            
        except Exception as e:
            logger.error(f"Failed to send immediate message to {phone_number}: {str(e)}")
            # Try alternative method if default fails
            try:
                logger.info("Trying alternative method...")
                return self._send_alternative(phone_number, message)
            except Exception as e2:
                logger.error(f"Alternative method also failed: {str(e2)}")
                return False
    
    def _send_with_selenium(self, phone_number: str, message: str) -> bool:
        """
        Send a WhatsApp message using Selenium for more reliable automation
        
        Args:
            phone_number: Phone number with country code
            message: Message to send
            
        Returns:
            bool: True if message sent successfully, False otherwise
        """
        if not self._initialize_selenium():
            logger.error("Failed to initialize Selenium, falling back to alternative method")
            return self._send_alternative(phone_number, message)
            
        try:
            # Remove the plus sign for the URL
            if phone_number.startswith('+'):
                phone_number = phone_number[1:]
                
            # URL encode the message
            encoded_message = urllib.parse.quote(message)
            
            # Create the WhatsApp URL
            whatsapp_url = f"https://web.whatsapp.com/send?phone={phone_number}&text={encoded_message}"
            
            logger.info(f"Opening WhatsApp Web with Selenium: {whatsapp_url}")
            self.driver.get(whatsapp_url)
            
            # Wait for WhatsApp to load and for the chat to be ready
            # Look for the send button which appears when the page is fully loaded
            send_button_xpath = '//span[@data-icon="send"]'
            
            # Wait longer for the first load
            logger.info("Waiting for WhatsApp Web to load (this might take time for first connection)...")
            WebDriverWait(self.driver, 60).until(
                EC.presence_of_element_located((By.XPATH, send_button_xpath))
            )
            
            # Wait additional time for any potential popups or loading
            time.sleep(5)
            
            # Click the send button
            send_button = self.driver.find_element(By.XPATH, send_button_xpath)
            send_button.click()
            
            # Wait for message to be sent
            logger.info("Message sent, waiting for confirmation...")
            time.sleep(5)
            
            # Close browser if required
            if self.close_tab:
                self._close_selenium()
                
            logger.info(f"Message successfully sent to {phone_number} using Selenium")
            return True
            
        except Exception as e:
            logger.error(f"Error sending message with Selenium: {str(e)}")
            # Try to close the browser to prevent resource leaks
            self._close_selenium()
            
            # Fall back to alternative method
            logger.info("Falling back to alternative method")
            return self._send_alternative(phone_number, message)
            
    def _send_alternative(self, phone_number: str, message: str) -> bool:
        """
        Alternative method to send WhatsApp messages using direct URL access
        
        Args:
            phone_number: Phone number with country code
            message: Message to send
            
        Returns:
            bool: True if message sent successfully, False otherwise
        """
        try:
            # Clean the phone number
            if phone_number.startswith('+'):
                phone_number = phone_number[1:]
                
            # URL encode the message
            encoded_message = urllib.parse.quote(message)
            
            # Create the WhatsApp URL
            whatsapp_url = f"https://web.whatsapp.com/send?phone={phone_number}&text={encoded_message}"
            
            logger.info(f"Opening WhatsApp Web directly: {whatsapp_url}")
            
            # Open the URL in default browser
            webbrowser.open(whatsapp_url)
            
            # Wait for WhatsApp Web to load (give it plenty of time)
            logger.info("Waiting for WhatsApp Web to load...")
            time.sleep(max(30, self.wait_time))  # Ensure at least 30 seconds wait
            
            # Try to press Enter to send the message
            logger.info("Attempting to send message via keyboard press...")
            pyautogui.press('enter')
            
            # Wait for message to send
            time.sleep(5)
            
            logger.info(f"Alternative message sent to {phone_number}")
            return True
            
        except Exception as e:
            logger.error(f"Alternative method failed: {str(e)}")
            return False
    
    def _format_phone_number(self, phone_number: str) -> str:
        """
        Format phone number to required format
        
        Args:
            phone_number: Input phone number
            
        Returns:
            str: Formatted phone number with country code
        """
        # Remove any non-digit characters
        cleaned = ''.join(filter(str.isdigit, phone_number))
        
        # Ensure it has country code (+ sign)
        if not phone_number.startswith('+'):
            cleaned = '+' + cleaned
            
        return cleaned

# Example usage
if __name__ == "__main__":
    # Create messenger instance with extended wait time
    messenger = WhatsAppMessenger(wait_time=30, close_tab=True, use_selenium=True)
    
    # Example for IoT temperature alert
    temperature = 38.5  # This would come from your sensor
    threshold = 35.0
    
    if temperature > threshold:
        # Phone number with country code (e.g., +1 for USA)
        recipient = "+919080610799"  # Replace with actual number
        
        # Compose alert message
        alert_message = f"""
        🚨 IoT TEMPERATURE ALERT 🚨
        
        Current temperature: {temperature}°C
        Threshold: {threshold}°C
        
        Action required: Check cooling system
        Device ID: TempSensor01
        Location: Server Room
        
        Alert time: {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
        """
        
        # Send message instantly
        success = messenger.send_message_instantly(recipient, alert_message)
        
        if success:
            print("Alert sent successfully!")
        else:
            print("Failed to send alert!")