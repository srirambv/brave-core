/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/net/brave_site_hacks_network_delegate_helper.h"

#include "brave/browser/net/url_context.h"
#include "brave/common/network_constants.h"
#include "chrome/test/base/chrome_render_view_host_test_harness.h"
#include "net/traffic_annotation/network_traffic_annotation_test_helper.h"
#include "net/url_request/url_request_test_util.h"

namespace {

class BraveSiteHacksNetworkDelegateHelperTest: public testing::Test {
 public:
  BraveSiteHacksNetworkDelegateHelperTest()
      : thread_bundle_(content::TestBrowserThreadBundle::IO_MAINLOOP),
        context_(new net::TestURLRequestContext(true)) {
  }
  ~BraveSiteHacksNetworkDelegateHelperTest() override {}
  void SetUp() override {}
  net::TestURLRequestContext* context() { return context_.get(); }

 private:
  content::TestBrowserThreadBundle thread_bundle_;
  std::unique_ptr<net::TestURLRequestContext> context_;
};


TEST_F(BraveSiteHacksNetworkDelegateHelperTest, NoChangeURL) {
  net::TestDelegate test_delegate;
  GURL url("https://bradhatesprimes.brave.com/composite_numbers_ftw");
  std::unique_ptr<net::URLRequest> request =
      context()->CreateRequest(url, net::IDLE, &test_delegate,
                               TRAFFIC_ANNOTATION_FOR_TESTS);
  std::shared_ptr<brave::BraveURLRequestContext>
      url_context(new brave::BraveURLRequestContext());
  brave::ResponseCallback callback;
  GURL new_url;
  int ret =
    OnBeforeURLRequest_SiteHacksWork(request.get(), &new_url, callback,
        url_context);
  EXPECT_TRUE(new_url.is_empty());
  EXPECT_EQ(ret, net::OK);
}

TEST_F(BraveSiteHacksNetworkDelegateHelperTest, RedirectsToEmptyDataURLs) {
  std::vector<GURL> urls({
    GURL("https://sp1.nypost.com"),
    GURL("https://sp.nasdaq.com")
  });
  std::for_each(urls.begin(), urls.end(),
      [this](GURL url){
    net::TestDelegate test_delegate;
    std::unique_ptr<net::URLRequest> request =
        context()->CreateRequest(url, net::IDLE, &test_delegate,
                                 TRAFFIC_ANNOTATION_FOR_TESTS);
    std::shared_ptr<brave::BraveURLRequestContext>
        url_context(new brave::BraveURLRequestContext());
    brave::ResponseCallback callback;
    GURL new_url;
    int ret =
      OnBeforeURLRequest_SiteHacksWork(request.get(), &new_url, callback,
          url_context);
    EXPECT_EQ(ret, net::OK);
    EXPECT_STREQ(new_url.spec().c_str(), kEmptyDataURI);
  }); }

TEST_F(BraveSiteHacksNetworkDelegateHelperTest, RedirectsToStubs) {
  std::vector<GURL> urls({
    GURL(kGoogleTagManagerPattern),
    GURL(kGoogleTagServicesPattern)
  });
  std::for_each(urls.begin(), urls.end(),
      [this](GURL url){
    net::TestDelegate test_delegate;
    std::unique_ptr<net::URLRequest> request =
        context()->CreateRequest(url, net::IDLE, &test_delegate,
                                 TRAFFIC_ANNOTATION_FOR_TESTS);
    std::shared_ptr<brave::BraveURLRequestContext>
        url_context(new brave::BraveURLRequestContext());
    brave::ResponseCallback callback;
    GURL new_url;
    int ret =
      OnBeforeURLRequest_SiteHacksWork(request.get(), &new_url, callback,
          url_context);
    EXPECT_EQ(ret, net::OK);
    EXPECT_TRUE(new_url.SchemeIs("data"));
  });
}

TEST_F(BraveSiteHacksNetworkDelegateHelperTest, Blocking) {
  std::vector<GURL> urls({
    GURL("https://www.lesechos.fr/xtcore.js"),
    GURL("https://bradhatesprimes.y8.com/js/sdkloader/outstream.js")
  });
  std::for_each(urls.begin(), urls.end(),
      [this](GURL url){
    net::TestDelegate test_delegate;
    std::unique_ptr<net::URLRequest> request =
        context()->CreateRequest(url, net::IDLE, &test_delegate,
                                 TRAFFIC_ANNOTATION_FOR_TESTS);
    std::shared_ptr<brave::BraveURLRequestContext>
        url_context(new brave::BraveURLRequestContext());
    brave::ResponseCallback callback;
    GURL new_url;
    int ret =
      OnBeforeURLRequest_SiteHacksWork(request.get(), &new_url, callback,
          url_context);
    EXPECT_EQ(ret, net::ERR_ABORTED);
  });
}

TEST_F(BraveSiteHacksNetworkDelegateHelperTest, ForbesWithCookieHeader) {
  GURL url("https://www.forbes.com");
  net::TestDelegate test_delegate;
  std::unique_ptr<net::URLRequest> request =
      context()->CreateRequest(url, net::IDLE, &test_delegate,
                               TRAFFIC_ANNOTATION_FOR_TESTS);
  net::HttpRequestHeaders headers;
  headers.SetHeader(kCookieHeader, "name=value; name2=value2; name3=value3");
  std::shared_ptr<brave::BraveURLRequestContext>
      url_context(new brave::BraveURLRequestContext());
  brave::ResponseCallback callback;
  int ret = brave::OnBeforeStartTransaction_SiteHacksWork(request.get(), &headers,
      callback, url_context);
  std::string cookies;
  headers.GetHeader(kCookieHeader, &cookies);
  EXPECT_TRUE(cookies.find(std::string("; ") + kForbesExtraCookies) != std::string::npos);
  EXPECT_EQ(ret, net::OK);
}

TEST_F(BraveSiteHacksNetworkDelegateHelperTest, ForbesWithoutCookieHeader) {
  GURL url("https://www.forbes.com/prime_numbers/573259391");
  net::TestDelegate test_delegate;
  std::unique_ptr<net::URLRequest> request =
      context()->CreateRequest(url, net::IDLE, &test_delegate,
                               TRAFFIC_ANNOTATION_FOR_TESTS);
  net::HttpRequestHeaders headers;
  std::shared_ptr<brave::BraveURLRequestContext>
      url_context(new brave::BraveURLRequestContext());
  brave::ResponseCallback callback;
  int ret = brave::OnBeforeStartTransaction_SiteHacksWork(request.get(), &headers,
      callback, url_context);
  std::string cookies;
  headers.GetHeader(kCookieHeader, &cookies);
  EXPECT_TRUE(cookies.find(kForbesExtraCookies) != std::string::npos);
  EXPECT_EQ(ret, net::OK);
}

TEST_F(BraveSiteHacksNetworkDelegateHelperTest, NotForbesNoCookieChange) {
  GURL url("https://www.brave.com/prime_numbers/573259391");
  net::TestDelegate test_delegate;
  std::unique_ptr<net::URLRequest> request =
      context()->CreateRequest(url, net::IDLE, &test_delegate,
                               TRAFFIC_ANNOTATION_FOR_TESTS);
  net::HttpRequestHeaders headers;
  std::string expected_cookies = "name=value; name2=value2; name3=value3";
  headers.SetHeader(kCookieHeader, "name=value; name2=value2; name3=value3");
  std::shared_ptr<brave::BraveURLRequestContext>
      url_context(new brave::BraveURLRequestContext());
  brave::ResponseCallback callback;
  int ret = brave::OnBeforeStartTransaction_SiteHacksWork(request.get(), &headers,
      callback, url_context);
  std::string cookies;
  headers.GetHeader(kCookieHeader, &cookies);
  EXPECT_STREQ(cookies.c_str(), expected_cookies.c_str());
  EXPECT_EQ(ret, net::OK);
}

TEST_F(BraveSiteHacksNetworkDelegateHelperTest, NoScriptTwitterMobileRedirect) {
  GURL url("https://mobile.twitter.com/i/nojs_router?path=%2F");
  net::TestDelegate test_delegate;
  std::unique_ptr<net::URLRequest> request =
      context()->CreateRequest(url, net::IDLE, &test_delegate,
                               TRAFFIC_ANNOTATION_FOR_TESTS);
  net::HttpRequestHeaders headers;
  headers.SetHeader(kRefererHeader, "https://twitter.com/");
  std::shared_ptr<brave::BraveURLRequestContext>
      url_context(new brave::BraveURLRequestContext());
  brave::ResponseCallback callback;
  int ret = brave::OnBeforeStartTransaction_SiteHacksWork(request.get(), &headers,
      callback, url_context);
  EXPECT_EQ(ret, net::ERR_ABORTED);
}

TEST_F(BraveSiteHacksNetworkDelegateHelperTest, AllowTwitterMobileRedirectFromDiffSite) {
  GURL url("https://mobile.twitter.com/i/nojs_router?path=%2F");
  net::TestDelegate test_delegate;
  std::unique_ptr<net::URLRequest> request =
      context()->CreateRequest(url, net::IDLE, &test_delegate,
                               TRAFFIC_ANNOTATION_FOR_TESTS);
  net::HttpRequestHeaders headers;
  headers.SetHeader(kRefererHeader, "https://brianbondy.com/");
  std::shared_ptr<brave::BraveURLRequestContext>
      url_context(new brave::BraveURLRequestContext());
  brave::ResponseCallback callback;
  int ret = brave::OnBeforeStartTransaction_SiteHacksWork(request.get(), &headers,
      callback, url_context);
  EXPECT_EQ(ret, net::OK);
}

TEST_F(BraveSiteHacksNetworkDelegateHelperTest, TwitterNoCancelWithReferer) {
  GURL url("https://twitter.com/brianbondy");
  net::TestDelegate test_delegate;
  std::unique_ptr<net::URLRequest> request =
      context()->CreateRequest(url, net::IDLE, &test_delegate,
                               TRAFFIC_ANNOTATION_FOR_TESTS);
  net::HttpRequestHeaders headers;
  headers.SetHeader(kRefererHeader, "https://twitter.com/");
  std::shared_ptr<brave::BraveURLRequestContext>
      url_context(new brave::BraveURLRequestContext());
  brave::ResponseCallback callback;
  int ret = brave::OnBeforeStartTransaction_SiteHacksWork(request.get(), &headers,
      callback, url_context);
  EXPECT_EQ(ret, net::OK);
}


}  // namespace