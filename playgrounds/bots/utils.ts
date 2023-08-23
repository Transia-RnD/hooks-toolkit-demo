import {
  Client,
  Wallet,
  Payment,
  TrustSet,
  xrpToDrops,
  OfferCreate,
} from '@transia/xrpl'
import { IssuedCurrencyAmount } from '@transia/xrpl/dist/npm/models/common'

import { Xrpld } from '@transia/hooks-toolkit'

// NOT EXPORTED
import {
  IC,
  ICXRP,
  XrplIntegrationTestContext,
  accountSet,
  fund,
  setupClient,
} from '@transia/hooks-toolkit/dist/npm/src/libs/xrpl-helpers'

// const lowChange = -0.5
// const highChange = 0.5
export function walkPrice(pv: number, hc: number, lc: number) {
  const change = (Math.random() * (hc - lc) + lc).toFixed(2)
  const newValue = (pv + parseFloat(change)).toFixed(2)
  return newValue
}

async function fundICWallet(
  client: Client,
  mwallet: Wallet,
  wallet: Wallet,
  ic: IC
) {
  const payTx: Payment = {
    TransactionType: 'Payment',
    Account: mwallet.classicAddress,
    Destination: wallet.classicAddress,
    Amount: xrpToDrops(2000),
  }
  await Xrpld.submit(client, {
    wallet: mwallet,
    tx: payTx,
  })
  const amount: IssuedCurrencyAmount = {
    value: String(100000000),
    currency: ic.currency as string,
    issuer: ic.issuer as string,
  }
  const trustTx: TrustSet = {
    TransactionType: 'TrustSet',
    Account: wallet.classicAddress,
    LimitAmount: amount,
  }
  await Xrpld.submit(client, {
    wallet: wallet,
    tx: trustTx,
  })
}

export async function sell(
  client: Client,
  wallet: Wallet,
  takerGets: IssuedCurrencyAmount,
  takerPays: string
) {
  const sellTx: OfferCreate = {
    TransactionType: 'OfferCreate',
    Account: wallet.classicAddress,
    TakerGets: takerGets,
    TakerPays: takerPays,
  }
  await Xrpld.submit(client, {
    wallet: wallet,
    tx: sellTx,
  })
}

export async function buy(
  client: Client,
  wallet: Wallet,
  takerGets: string,
  takerPays: IssuedCurrencyAmount
) {
  const buyTx: OfferCreate = {
    TransactionType: 'OfferCreate',
    Account: wallet.classicAddress,
    TakerGets: takerGets,
    TakerPays: takerPays,
  }
  await Xrpld.submit(client, {
    wallet: wallet,
    tx: buyTx,
  })
}

export async function init(seed: string, currency: string) {
  const client = new Client('wss://hooks-testnet-v3.xrpl-labs.com')
  await client.connect()
  client.networkID = await client.getNetworkID()

  const masterWallet = await client.fundWallet(null, {
    faucetHost: 'hooks-testnet-v3.xrpl-labs.com',
  })
  const gwWallet = Wallet.fromSeed(seed)
  const oneWallet = Wallet.fromSeed('snQo9w7ZjdiYfS3tUUXab73VTuqwT')
  const twoWallet = Wallet.fromSeed('shJgpGURyv2vVEHNCqMq7ujbR7Sku')
  // const carolWallet = testContext.carol

  const ic = IC.gw(currency, gwWallet.classicAddress)
  await fund(
    client,
    masterWallet.wallet,
    new ICXRP(3000),
    ...[gwWallet.classicAddress]
  )
  await accountSet(client, gwWallet)

  // FUND BUYER WALLET
  await fundICWallet(client, masterWallet.wallet, oneWallet, ic)

  // FUND SELLER WALLET
  await fundICWallet(client, masterWallet.wallet, twoWallet, ic)

  // FUND 3RD PARTY WALLET
  // await fundICWallet(testContext, carolWallet, ic)

  await client.disconnect()
}
