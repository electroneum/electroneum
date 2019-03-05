// Copyrights(c) 2017-2018, The Electroneum Project

#include "gtest/gtest.h"

#include "crypto/crypto.h"

std::vector<std::string> keypair = crypto::create_ed25519_keypair();
std::string privateKey = boost::algorithm::unhex(keypair[0]);
std::string publicKey = boost::algorithm::unhex(keypair[1]);
std::string message("Message requiring signing");
std::string unusedMessage("Unused message");
std::string validSignature = crypto::sign_message(message, privateKey);
std::string invalidSignature = crypto::sign_message(message, keypair[0]);
std::string blankInvalidSignature = crypto::sign_message(message, "");

TEST(ed25519_signature, CreateKeyPair)
{
    ASSERT_EQ(keypair.size(), 2);
    ASSERT_EQ(keypair[0].size(), 64);
    ASSERT_EQ(keypair[1].size(), 64);
    ASSERT_EQ(privateKey.size(), 32);
    ASSERT_EQ(publicKey.size(), 32);
}

TEST(DelegatedPOW, SignMessage)
{
    ASSERT_EQ(validSignature.size(), 64);
    ASSERT_EQ(invalidSignature.size(), 0);
    ASSERT_EQ(blankInvalidSignature.size(), 0);
}

TEST(DelegatedPOW, VerifySignature)
{
    EXPECT_TRUE(crypto::verify_signature(message, publicKey, validSignature));
    EXPECT_FALSE(crypto::verify_signature(message, privateKey, validSignature));
    EXPECT_FALSE(crypto::verify_signature(unusedMessage, publicKey, validSignature));
    EXPECT_FALSE(crypto::verify_signature(message, publicKey, invalidSignature));
    EXPECT_FALSE(crypto::verify_signature(message, privateKey, invalidSignature));
    EXPECT_FALSE(crypto::verify_signature(unusedMessage, publicKey, invalidSignature));
}