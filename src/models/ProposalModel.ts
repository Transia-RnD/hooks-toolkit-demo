import {
  BaseModel,
  Metadata,
  UInt8,
  UInt64,
  XFL,
  Currency,
  XRPAddress,
} from '@transia/hooks-toolkit/dist/npm/src/libs/binary-models'

export class ProposalModel extends BaseModel {
  expiration: UInt64 // Ledger Sequence
  orderType: UInt8 // 0 BUY 1 SELL
  value: XFL // Amount as XFL
  currency: Currency // Currency
  issuer: XRPAddress // Issuer

  constructor(
    expiration: UInt64,
    orderType: UInt8,
    value: XFL,
    currency: Currency,
    issuer: XRPAddress
  ) {
    super()
    this.expiration = expiration
    this.orderType = orderType
    this.value = value
    this.currency = currency
    this.issuer = issuer
  }

  getMetadata(): Metadata {
    return [
      { field: 'expiration', type: 'uint64' },
      { field: 'orderType', type: 'uint8' },
      { field: 'value', type: 'xfl' },
      { field: 'currency', type: 'currency' },
      { field: 'issuer', type: 'xrpAddress' },
    ]
  }

  toJSON() {
    return {
      expiration: this.expiration,
      orderType: this.orderType,
      value: this.value,
      currency: this.currency,
      issuer: this.issuer,
    }
  }
}
