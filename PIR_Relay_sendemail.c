/***************************************************************************
 * PIR_Relay_sendemail.c
 * 
 * 在 PIR 觸發時發送電子郵件
 *  
 * Compile: $ sudo gcc PIR_Relay_sendemail.c -l rt -l bcm2835 -l curl -std=gnu99 -o PIR_Relay_sendemail
 * 
 * Note: 編譯時若找不到 curl/curl.h 就安裝 libcurl4-openssl-dev 後再編譯一次
 *  $ sudo apt-get install libcurl4-openssl-dev
 * 
 *  
 ***************************************************************************/
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>  // libcurl 標頭檔
#include <bcm2835.h>    // RPi GPIO 函式庫

// libcurl - 定義電子郵件發送資料
#define GMAIL_USER  "putyourusername"       // GMail 帳號
#define GMAIL_PASS  "putyourpasswordhere"   // GMail 密碼
#define TO          "<your@email.address>"  // GMail 信箱
// 傳送給何人
// define CC          "<another@mail.com>"      // 如果有副本寄件人的話就把前面的雙斜線拿掉
#define SUBJECT     "Intrusion!!"
#define TEXT        "Your PIR sensor detected movement"

// bcm2835 - 定義輸入接腳號碼
#define PIN_PIR     18
#define STB_TIME    3
#define PIN_RELAY   23  // 定義樹莓派控制繼電器模組的接腳

// libcurl - 訊息內容
static const char *payload_text[]={
  "To: " TO "\n",
  "From: " GMAIL_USER "\n",
  //"Cc: " CC "\n",     // 如果有副本寄件人的話就把前面的雙斜線拿掉
  "Subject: " SUBJECT "\n",
  "\n", /* empty line to divide headers from body, see RFC5322 */
  // 下面開始就是文件內文  
  //"The body of the message starts here.\n",
  //"\n",
  //"It could be a lot of lines, could be MIME encoded, whatever.\n",
  //"Check RFC5322.\n",
  TEXT,
  NULL    
};

struct upload_status {
  int lines_read;
};
 
static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{
  struct upload_status *upload_ctx = (struct upload_status *)userp;
  const char *data;
 
  if ((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
    return 0;
  }
 
  data = payload_text[upload_ctx->lines_read];
 
  if (data) {
    size_t len = strlen(data);
    memcpy(ptr, data, len);
    upload_ctx->lines_read ++;
    return len;
  }
  return 0;
}

void send_email(void)
{
    CURL *curl;
    CURLcode res;
    struct curl_slist *recipients = NULL;
    struct upload_status upload_ctx;
    
    upload_ctx.lines_read = 0;
    
    curl = curl_easy_init();
    if (curl) {
        /* 設定郵件伺服器，請注意到網址後面要加上 587 取代 SMTP 正常使用的 PORT 25
         * ，Port 587 是常用於安全郵件的提交, 但是這裡需視所使用的郵件伺服器加以設定
         *  以符合自己的情形 */ 
        curl_easy_setopt(curl, CURLOPT_URL, "smtp://smtp.gmail.com:587");
        
        /* In this example, we'll start with a plain text connection, and upgrade
         * to Transport Layer Security (TLS) using the STARTTLS command. Be careful
         * of using CURLUSESSL_TRY here, because if TLS upgrade fails, the transfer
         * will continue anyway - see the security discussion in the libcurl
         * tutorial for more details. */ 
        curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
        
        /* 因為是使用 Gmail 伺服器所以要登入之後才能使用*/ 
        curl_easy_setopt(curl, CURLOPT_USERNAME, GMAIL_USER);   // GMAIL_USER
        curl_easy_setopt(curl, CURLOPT_PASSWORD, GMAIL_PASS);   // GMAIL_PASS
        
        /* value for envelope reverse-path */ 
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, GMAIL_USER);  // GMAIL_USER
        /* Add two recipients, in this particular case they correspond to the
         * To: and Cc: addressees in the header, but they could be any kind of
         * recipient. */ 
        recipients = curl_slist_append(recipients, TO);
        //recipients = curl_slist_append(recipients, CC);   // 如果有副本寄件人的話就把前面的雙斜線拿掉
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
        
        /* In this case, we're using a callback function to specify the data. You
         * could just use the CURLOPT_READDATA option to specify a FILE pointer to
         * read from.
         */ 
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
        curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
        
        /* Since the traffic will be encrypted, it is very useful to turn on debug
         * information within libcurl to see what is happening during the transfer.
         */ 
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        
        /* send the message (including headers) */ 
        res = curl_easy_perform(curl);
        /* Check for errors */ 
        if(res != CURLE_OK)
          fprintf(stderr, "curl_easy_perform() failed: %s\n",
                  curl_easy_strerror(res));
        
        /* free the list of recipients and clean up */ 
        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);
    }
    printf("\n\n");
}

int main(void)
{
    // bcm2835 函式庫初始化
    if(!bcm2835_init())
        return 1;

    // IO 初始化與初始狀態設定
    bcm2835_gpio_fsel(PIN_PIR, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(PIN_RELAY, BCM2835_GPIO_FSEL_OUTP);   //定義連接繼電器的接腳為輸出
    // pull-down the control pin of PIR
    bcm2835_gpio_set_pud(PIN_PIR, BCM2835_GPIO_PUD_DOWN);
    bcm2835_gpio_write(PIN_RELAY, HIGH);                     // 設定繼電器初始狀態
    
    // PIR 接上電源後須要等待 20 秒才會穩定，穩定時間內會有不穩定的輸出，
    // 因此不需要偵測此時的 PIR 狀態
    printf("\n");
    for(int i = 0; i < STB_TIME; i++)
    {
        bcm2835_delay(1000);
        printf("      Waitting for PIR to stable, %d sec\r", STB_TIME - i);
        fflush(stdout);
    }

    printf("\n\n  READY ...\n\n");

    while(1)
    {
        // 現在可以用手或是身體在 PIR 前面晃動
        if(bcm2835_gpio_lev(PIN_PIR))
        {
            printf("THE PIR SENSOR DETECTED MOVEMENT           \r\n\n");            
            fflush(stdout);
            send_email();
            bcm2835_gpio_write(PIN_RELAY, LOW); // 繼電器輸出
            bcm2835_delay(3000);            // 此時間的設定必須要大於 PIR 的延遲時間
        }
        else
        {
            bcm2835_gpio_write(PIN_RELAY, HIGH);  // 繼電器輸出
            printf("PIR sensor get ready for movement detection\r");
            fflush(stdout);
        }            
    }
    
    // 關閉 bcm2835 函式庫
    bcm2835_close();
    return 0;
}